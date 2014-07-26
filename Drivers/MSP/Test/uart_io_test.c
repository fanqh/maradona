#include <string.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "errno_ex.h"
#include "uart_io.h"
#include "uart_io_private.h"

typedef struct 
{
	/** simulated registers **/
	uint32_t 								rx_m0ar;
	int 										rx_ndtr;
	
	uint32_t								tx_m0ar;
 	int											tx_ndtr;
	
	char*										rxdma_buf;
	char*										txdma_buf;
	
} uio_testdata_t;

/*** hal apis that will be mocked ***/
const usart_api_t usart_apis_default =
{
	.HAL_UART_Init = HAL_UART_Init,
	.HAL_UART_Transmit_DMA = HAL_UART_Transmit_DMA,
	.HAL_UART_Receive_DMA = HAL_UART_Receive_DMA,	
	.UART_IO_RxFlipBuffer = UART_IO_RxFlipBuffer
};

usart_api_t usart_apis;

/** mock begin **/
static char rxbuf[2][UART_IO_BUFFER_SIZE];
static char txbuf[2][UART_IO_BUFFER_SIZE];
static char tx_dma_buffer[UART_IO_BUFFER_SIZE];

static 	UART_HandleTypeDef			m_huart_default =
{
	.State = HAL_UART_STATE_RESET,
};

static UARTEX_HandleTypeDef			m_huartex_default =
{
	.huart = 
	{
		.State = HAL_UART_STATE_RESET,
	}
};
		
static 	UART_HandleTypeDef 			m_huart;
static	UART_IO_HandleTypeDef 	m_huio_default = 
{
	.handle = &m_huartex_default,
	.rbuf[0] = rxbuf[0],
	.rbuf[1] = rxbuf[1],
	.rx_upper = rxbuf[1],
	.rx_head = &rxbuf[1][UART_IO_BUFFER_SIZE],
	.rx_tail = &rxbuf[1][UART_IO_BUFFER_SIZE],
	
	.tbuf[0] = txbuf[0],
	.tbuf[1] = txbuf[1],

	.tx_head = &txbuf[1][0],
	.tx_tail = &txbuf[1][0]
};

static	UART_IO_HandleTypeDef 	m_huio;

static 	uint32_t								m_tx_m0ar;
static 	int											m_tx_ndtr;

// extern 	HAL_StatusTypeDef __uart_io_rx_flip_buffer(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
//static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_ok(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
//static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_error(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
//static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_timeout(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
//static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_busy(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);


/** utils begin **/
#define			WHATEVER						1234
#define 		TEST_BUFFER_SIZE 		1024
static 			char 								test_buffer[TEST_BUFFER_SIZE];

const static char literal[] = 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789" 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789" 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"			 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789";

const char* literal_end = literal + sizeof(literal);


HAL_StatusTypeDef m_init_hal_ok(UART_HandleTypeDef *huart) {
	
	TEST_ASSERT_EQUAL(HAL_UART_STATE_RESET, huart->State);
	return HAL_OK;
}

HAL_StatusTypeDef m_init_hal_err(UART_HandleTypeDef *huart) {
	
	return HAL_ERROR;
}

// HAL_StatusTypeDef (*transmit)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
//HAL_StatusTypeDef m_receive_hal_ok(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) 
//{
//	m_rx_m0ar = (uint32_t)pData;
//	m_rx_ndtr = Size;
//	
//	return HAL_OK;
//}

// HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
static HAL_StatusTypeDef m_transmit_hal_ok(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	memset(tx_dma_buffer, 0, UART_IO_BUFFER_SIZE);
	memmove(tx_dma_buffer, pData, Size);
	
	m_tx_m0ar = (uint32_t)pData;
	m_tx_ndtr = Size;
	return HAL_OK;
}

static HAL_StatusTypeDef m_transmit_hal_busy(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	return HAL_BUSY;
}



static void fill_rx_testdata(	UART_IO_HandleTypeDef* huio, 	/** handle 								**/
															int index,										/** upper									**/
															int offset,										/** upper offset 					**/
															const char* upper,						/** content for upper 		**/
															int upper_size,								/** upper buffer fillsize **/
															const char* dma,							/** dma buffer 						**/
															int dma_size,									/** size to copy into dma buffer **/
															int rx_size)									/** previously set dma recv size **/
{
	int other;
	uio_testdata_t* td = huio->handle->test_data;
	
	huio->rx_upper = huio->rbuf[index];
	huio->rx_head = &huio->rx_upper[offset];
	huio->rx_tail = huio->rx_head + upper_size;
	
	if (upper_size)
	{
		memmove(huio->rx_head, upper, upper_size);
	}
	
	other = (index == 0) ? 1 : 0;
	
	td->rx_m0ar = (uint32_t)huio->rbuf[other];
	td->rx_ndtr = rx_size - dma_size;
	td->rxdma_buf = huio->rbuf[other];
	
	if (dma_size)
	{
		memmove(td->rxdma_buf, dma, dma_size);
	}
}

static void fill_tx_upper(int index, const char* src, int size)
{
	TEST_ASSERT_TRUE(size < UART_IO_BUFFER_SIZE);
	
	m_huio.tx_head = m_huio.tbuf[index];
	memmove(m_huio.tx_head, src, size);
	m_huio.tx_tail = m_huio.tx_head + size;
}

static void fill_tx_testdata( UART_IO_HandleTypeDef* huio,
															int index,
															const char* src,
															int upper_size,
															const char* dma,							/** dma buffer 						**/
															int dma_size,									/** size to copy into dma buffer **/
															int tx_sent)									/** previously set dma recv size **/
{
	/** dma and dma_size should be considered as the arguments previously 
			passed to HAL_UART_Transmit_DMA, and tx_sent should be 'expected' number of bytes 
			already sent. **/
	
	int other;
	uio_testdata_t* td = huio->handle->test_data;
	
	huio->tx_head = huio->tbuf[index];
	huio->tx_tail = huio->tx_head + upper_size;
	
	if (upper_size)
	{
		memmove(huio->tx_head, src, upper_size);
	}
	
	other = (index == 0) ? 1 : 0;
	
	td->tx_m0ar = (uint32_t)huio->tbuf[other];
	td->txdma_buf = huio->tbuf[other];
	
	if (dma_size)
	{
		memmove(td->txdma_buf, dma, dma_size);
	}
	
	td->tx_ndtr = dma_size - tx_sent;
}
															

/** set and fill dma buffer **/
/** dst is set as dma buffer **/
//static void fill_dma_buffer(char* dst, const char* src, int size)
//{
//	TEST_ASSERT_NOT_EQUAL(0, dst);
//	TEST_ASSERT_NOT_EQUAL(0, src);
//	TEST_ASSERT_NOT_EQUAL(0, size);
//	TEST_ASSERT_TRUE(size < UART_IO_BUFFER_SIZE);
//	
//	m_rx_m0ar = (uint32_t)dst;
//	m_rx_ndtr = UART_IO_BUFFER_SIZE - size;
//	
//	memmove(dst, src, size);
//}

/** utils end **/



///////////////////////////////////////////////////////////////////////////////
// Read
//
TEST_GROUP(UsartIO_DMA);
TEST_SETUP(UsartIO_DMA)
{
	memset(test_buffer, 0, TEST_BUFFER_SIZE);
	memset(&m_huio, 0, sizeof(m_huio));
	memset(&m_huart, 0, sizeof(m_huart));
	
	m_huart = m_huart_default;
	m_huio = m_huio_default;
	
//	m_rx_m0ar = 0;
//	m_rx_ndtr = 0;
	
	usart_apis = usart_apis_default;
	
	// UARTEX_HandleTypeDef
}

TEST_TEAR_DOWN(UsartIO_DMA)
{
}

TEST(UsartIO_DMA, ReadInvalidArgs)
{
	char c;
	UART_IO_HandleTypeDef huio;
	
	/** These tests are enough, don't validate 'internal state' of opaque struct, pointless. **/
	TEST_ASSERT_EQUAL(-1, UART_IO_Read(0, &c, 1));					/** null handle **/
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(-1, UART_IO_Read(&huio, 0, 1));				/** null buf p	**/
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(0, UART_IO_Read(&huio, &c, 0));				/** no error checking as linux **/
}

TEST(UsartIO_DMA, ReadWhenBytesToReadLessThanOrEqualToBytesInBuffer)
{
	char sample[] = "test";
	char buf[64];
	int read;
	
	UART_IO_HandleTypeDef huio;	
	UARTEX_HandleTypeDef hue;
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;
	
	// fill_rx_upper(&huio, 1, 1, sample, strlen(sample));	/** use buffer 1 as upper buffer **/
	fill_rx_testdata(&huio, 1, 1, sample, strlen(sample), 0, 0, 0);
	
	memset(buf, 0, sizeof(buf));	
	read = UART_IO_Read(&huio, buf, strlen(sample));
	
	TEST_ASSERT_EQUAL(strlen(sample), read);
	TEST_ASSERT_EQUAL_MEMORY(sample, buf, strlen(sample));
}


HAL_StatusTypeDef swap_mock_hal_ok(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	uio_testdata_t* test_data = (uio_testdata_t*)h->test_data;
	
	if (m0ar) *m0ar = test_data->rx_m0ar;
	if (ndtr) *ndtr = test_data->rx_ndtr;
	
	test_data->rx_m0ar = (uint32_t)buf;
	test_data->rx_ndtr = size;
	
	return HAL_OK;
}

TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalReady)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;

	UART_IO_HandleTypeDef huio;
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
	UARTEX_Operations uart_ops;
	
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;

	memset(&uart_ops, 0, sizeof(uart_ops));
	uart_ops.swap = swap_mock_hal_ok;
	huio.handle->ops = &uart_ops;	
	
	memset(&td, 0, sizeof(td));	
	huio.handle->test_data = &td;
	fill_rx_testdata(&huio, 0, 1, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);
	
	memset(buf, 0, sizeof(buf));
	read = UART_IO_Read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft) + strlen(hard), read);
	TEST_ASSERT_EQUAL_STRING("testdriven", buf);
}

static HAL_StatusTypeDef swap_mock_hal_error(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_ERROR;
}

static HAL_StatusTypeDef swap_mock_hal_busy(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_BUSY;
}

static HAL_StatusTypeDef swap_mock_hal_timeout(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_TIMEOUT;
}

TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferNotEmpty)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;

	UART_IO_HandleTypeDef huio;
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
	UARTEX_Operations uart_ops;
	
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;

	memset(&uart_ops, 0, sizeof(uart_ops));
	huio.handle->ops = &uart_ops;	
	
	memset(&td, 0, sizeof(td));	
	huio.handle->test_data = &td;
	
	// fill and mock
	fill_rx_testdata(&huio, 0, 1, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);	
	uart_ops.swap = swap_mock_hal_error; // swap_mock_hal_ok;
	
	memset(buf, 0, sizeof(buf));	
	read = UART_IO_Read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);
	
	// refill and remock
	fill_rx_testdata(&huio, 1, 1, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);	
	uart_ops.swap = swap_mock_hal_busy;
	
	memset(buf, 0, sizeof(buf));	
	read = UART_IO_Read(&huio, buf, sizeof(buf));

	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);
	
	// refill and remock
	fill_rx_testdata(&huio, 1, 2, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);	
	uart_ops.swap = swap_mock_hal_timeout;
	
	memset(buf, 0, sizeof(buf));	
	read = UART_IO_Read(&huio, buf, sizeof(buf));

	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);	
}

TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferEmpty)
{
	/** This case says if upper buffer is empty and HAL in error state, the function should return error **/
	char buf[64];
	int read;
	
	UART_IO_HandleTypeDef huio;
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
	UARTEX_Operations uart_ops;
	
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;

	memset(&uart_ops, 0, sizeof(uart_ops));
	huio.handle->ops = &uart_ops;	
	
	memset(&td, 0, sizeof(td));	
	huio.handle->test_data = &td;
	
	fill_rx_testdata(&huio, 1, 1, 0, 0, 0, 0, 0);
	huio.handle->ops->swap = swap_mock_hal_error;
	
	memset(buf, 0, sizeof(buf));
	read = UART_IO_Read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EINVAL, errno);	
	
	fill_rx_testdata(&huio, 1, 1, 0, 0, 0, 0, 0);
	huio.handle->ops->swap = swap_mock_hal_busy;
	
	memset(buf, 0, sizeof(buf));
	read = UART_IO_Read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EBUSY, errno);	

	fill_rx_testdata(&huio, 1, 1, 0, 0, 0, 0, 0);
	huio.handle->ops->swap = swap_mock_hal_timeout;
	
	memset(buf, 0, sizeof(buf));
	read = UART_IO_Read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EIO, errno);	
}

///////////////////////////////////////////////////////////////////////////////
#if 0 // open tests commented out
/******************************************************************************
 * 
 * Test Cases for USART_IO_Open
 *
 *****************************************************************************/

//TEST(UsartIO_DMA, OpenInvalidArgs)
//{
//	TEST_ASSERT_EQUAL(0, UART_IO_Open(0));
//	TEST_ASSERT_EQUAL(EINVAL, errno);
//	TEST_ASSERT_EQUAL(0, UART_IO_Open(7));
//	TEST_ASSERT_EQUAL(EINVAL, errno);
//}

//TEST(UsartIO_DMA, OpenPortUnavailable)	/** handle does not exist **/
//{
//	UART_IO_SetHandle(6, 0);
//	
//	TEST_ASSERT_EQUAL(0, UART_IO_Open(6));
//	TEST_ASSERT_EQUAL(ENODEV, errno);
//}

//TEST(UsartIO_DMA, OpenInvalidHandle)
//{
//	m_huio.handle = 0;
//	UART_IO_SetHandle(6, &m_huio);
//	TEST_ASSERT_EQUAL(0, UART_IO_Open(6));
//}

//TEST(UsartIO_DMA, OpenHalNotResetState)
//{
//	m_huio.handle->State = HAL_UART_STATE_READY;
//	
//	UART_IO_SetHandle(6, &m_huio);
//	
//	TEST_ASSERT_EQUAL(0, UART_IO_Open(6));
//	TEST_ASSERT_EQUAL(EBUSY, errno);
//}

//TEST(UsartIO_DMA, OpenHalInitOkReceiveOk)
//{
//	UART_IO_SetHandle(6, &m_huio);
//	usart_apis.HAL_UART_Init = m_init_hal_ok;
//	usart_apis.HAL_UART_Receive_DMA = m_receive_hal_ok;
//	
//	TEST_ASSERT_EQUAL(&m_huio, UART_IO_Open(6));
//	
//	/** should we check HAL state? I don't think so, cause
//	the function trust the return value, rather than 
//	checking state **/
//	
//	/** all following should be checked since they ARE
//	the results of action of the function under test **/
//	TEST_ASSERT_TRUE(m_rx_m0ar == (uint32_t)m_huio.rbuf[0]);
//	TEST_ASSERT_TRUE(m_rx_ndtr == UART_IO_BUFFER_SIZE);
//	
//	TEST_ASSERT_TRUE(m_huio.rx_upper == m_huio.rbuf[1]);
//	TEST_ASSERT_TRUE(m_huio.rx_tail == &m_huio.rx_upper[UART_IO_BUFFER_SIZE]);
//	TEST_ASSERT_TRUE(m_huio.rx_head == m_huio.rx_tail);
//	
//	TEST_ASSERT_TRUE(m_huio.tx_head == &m_huio.tbuf[1][0]);
//	TEST_ASSERT_TRUE(m_huio.tx_tail == m_huio.tx_head);
//}
#endif

///////////////////////////////////////////////////////////////////////////////
TEST(UsartIO_DMA, WriteInvalidArgs)
{
	char c;
	UART_IO_HandleTypeDef huio;
	
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(0, &c, 1));
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&huio, 0, 1));
	TEST_ASSERT_EQUAL(EINVAL, errno);
	
	/** no effect for errno**/
	/** may be changed future, man page write **/
	TEST_ASSERT_EQUAL(0, UART_IO_Write(&huio, &c, 0)); 
}

TEST(UsartIO_DMA, WriteHalStateTimeoutErrorReset)
{
	// HAL_UART_STATE_TIMEOUT means hardware error.
	// HAL_UART_STATE_RESET should not happen.
	// HAL_UART_STATE_ERROR are not used in firmware 1.1
	char c;
	UARTEX_HandleTypeDef huartex;
	UART_IO_HandleTypeDef huio;
	huio.handle = &huartex;
	
	errno = 0;
	huio.handle->huart.State = HAL_UART_STATE_TIMEOUT;
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);
	
	errno = 0;
	huio.handle->huart.State = HAL_UART_STATE_RESET;
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);

	errno = 0;
	huio.handle->huart.State = HAL_UART_STATE_ERROR;
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);
}


static HAL_StatusTypeDef send_mock_hal_ok(UARTEX_HandleTypeDef *huartex, uint8_t *pData, uint16_t Size)
{
	uio_testdata_t* td = huartex->test_data;
	
//	memset(tx_dma_buffer, 0, UART_IO_BUFFER_SIZE);
//	memmove(tx_dma_buffer, pData, Size);
	
	memset(td->txdma_buf, 0, UART_IO_BUFFER_SIZE);
	memmove(td->txdma_buf, pData, Size);
		
//	m_tx_m0ar = (uint32_t)pData;
//	m_tx_ndtr = Size;
	
	td->tx_m0ar = (uint32_t)pData;
	td->tx_ndtr = Size;
	
	return HAL_OK;
}

static HAL_StatusTypeDef send_mock_hal_busy(UARTEX_HandleTypeDef *huartex, uint8_t *pData, uint16_t Size)
{
	return HAL_BUSY;
}

TEST(UsartIO_DMA, WriteBufferSpaceAdequateAndHalReady)	// write 
{
	char s1[] = "test";
	char s2[] = "driven";
	char s3[] = "testdriven";
	int written;
	
	///////////////////////////
	UART_IO_HandleTypeDef huio;
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
	UARTEX_Operations uart_ops;
	
	char txbuf0[64];
	char txbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.tbuf[0] = txbuf0;
	huio.tbuf[1] = txbuf1;

	memset(&uart_ops, 0, sizeof(uart_ops));
	huio.handle->ops = &uart_ops;	
	
	memset(&td, 0, sizeof(td));	
	huio.handle->test_data = &td;	
	//////////////////////////////
	
	// HAL_UART_Transmit_DMA

	fill_tx_testdata(&huio, 1, s1, strlen(s1), 0, 0, 0);
	uart_ops.send = send_mock_hal_ok;
	huio.handle->huart.State = HAL_UART_STATE_READY;
	
	written = UART_IO_Write(&huio, s2, strlen(s2));
	
	TEST_ASSERT_EQUAL(strlen(s2), written);
	TEST_ASSERT_EQUAL_MEMORY(s3, td.txdma_buf, strlen(s3));
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[1], (char*)td.tx_m0ar);
	TEST_ASSERT_EQUAL(strlen(s3), td.tx_ndtr);
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[0], huio.tx_head);
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[0], huio.tx_tail);
	
}

TEST(UsartIO_DMA, WriteBufferSpaceAdequateAndHalBusy)
{
	char s1[] = "test";
	char s2[] = "driven";
	char s3[] = "testdriven";
	int written;
	
	///////////////////////////
	UART_IO_HandleTypeDef huio;
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
	UARTEX_Operations uart_ops;
	
	char txbuf0[64];
	char txbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.tbuf[0] = txbuf0;
	huio.tbuf[1] = txbuf1;

	memset(&uart_ops, 0, sizeof(uart_ops));
	huio.handle->ops = &uart_ops;	
	
	memset(&td, 0, sizeof(td));	
	huio.handle->test_data = &td;	
	//////////////////////////////

	uart_ops.send = send_mock_hal_busy;
	hue.huart.State = HAL_UART_STATE_READY;
	fill_tx_testdata(&huio, 1, s1, strlen(s1), 0, 0, 0);
	
	written = UART_IO_Write(&huio, s2, strlen(s2));
	
	TEST_ASSERT_EQUAL(strlen(s2), written);
	TEST_ASSERT_EQUAL_MEMORY(s3, huio.tbuf[1], strlen(s3));
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[1], huio.tx_head);
	TEST_ASSERT_EQUAL(strlen(s3), huio.tx_tail - huio.tx_head);
	
//	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_busy;
//	fill_tx_upper(1, s1, strlen(s1));
//	m_huio.handle->huart.State = HAL_UART_STATE_READY;	/** otherwise trapped **/
//	
//	written = UART_IO_Write(&m_huio, s2, strlen(s2));

//	TEST_ASSERT_EQUAL(strlen(s2), written);
//	
//	TEST_ASSERT_EQUAL_MEMORY(s3, m_huio.tbuf[1], strlen(s3));
//	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], m_huio.tx_head);
//	TEST_ASSERT_EQUAL(strlen(s3), m_huio.tx_tail - m_huio.tx_head);
}

TEST(UsartIO_DMA, WriteBufferSpaceInadequateAndHalReady)
{
	char s1[UART_IO_BUFFER_SIZE/2];
	char s2[UART_IO_BUFFER_SIZE]; 
	char s3[UART_IO_BUFFER_SIZE*2];
	int written;
	
	memset(s1, 'a', sizeof(s1));
	memset(s2, 'b', sizeof(s2));
	memset(s3, 0, sizeof(s3));
	memset(s3, 'a', sizeof(s1));
	memset(&s3[sizeof(s1)], 'b', sizeof(s2));
	
	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_ok;
	fill_tx_upper(1, s1, sizeof(s1));
	m_huio.handle->huart.State = HAL_UART_STATE_READY;	/** otherwise trapped **/
	
	written = UART_IO_Write(&m_huio, s2, sizeof(s2));

	TEST_ASSERT_EQUAL(sizeof(s2), written);
	
	TEST_ASSERT_EQUAL_MEMORY(s3, tx_dma_buffer, UART_IO_BUFFER_SIZE);
	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], (char*)m_tx_m0ar);
	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE, m_tx_ndtr);
	
	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[0], m_huio.tx_head);
	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE - UART_IO_BUFFER_SIZE/2, m_huio.tx_tail-m_huio.tx_head);
	TEST_ASSERT_EQUAL_MEMORY(&s3[UART_IO_BUFFER_SIZE], m_huio.tx_head, UART_IO_BUFFER_SIZE/2);
}

TEST(UsartIO_DMA, WriteBufferSpaceInadequateAndHalBusy)
{
	char s1[UART_IO_BUFFER_SIZE/2];
	char s2[UART_IO_BUFFER_SIZE]; 
	char s3[UART_IO_BUFFER_SIZE*2];
	int written;
	
	memset(s1, 'a', sizeof(s1));
	memset(s2, 'b', sizeof(s2));
	memset(s3, 0, sizeof(s3));
	memset(s3, 'a', sizeof(s1));
	memset(&s3[sizeof(s1)], 'b', sizeof(s2));
	
	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_busy;
	fill_tx_upper(1, s1, sizeof(s1));
	m_huio.handle->huart.State = HAL_UART_STATE_READY;	/** otherwise trapped **/
	
	written = UART_IO_Write(&m_huio, s2, sizeof(s2));

	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE - sizeof(s1), written);
	
	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], m_huio.tx_head);
	TEST_ASSERT_EQUAL_HEX32(&m_huio.tbuf[1][UART_IO_BUFFER_SIZE], m_huio.tx_tail);
	TEST_ASSERT_EQUAL_MEMORY(s3, m_huio.tx_head, UART_IO_BUFFER_SIZE);	
}


/******************************************************************************
 *
 * Group Runner
 *
 *****************************************************************************/
TEST_GROUP_RUNNER(UsartIO_DMA)
{
	// args invalid
	// args valid, read n bytes, n <= bytes in upper buffer
	// args valid, n > bytes in upper buffer, and HAL READY
	// args valid, n > bytes in upper buffer, and HAL not READY and upper buffer not empty
	// args valid, n > bytes in upper buffer, and HAL not READY adn upper buffer empty
	RUN_TEST_CASE(UsartIO_DMA, ReadInvalidArgs);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadLessThanOrEqualToBytesInBuffer);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferNotEmpty);	
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferEmpty);
	
//	RUN_TEST_CASE(UsartIO_DMA, OpenInvalidArgs);
//	RUN_TEST_CASE(UsartIO_DMA, OpenPortUnavailable);
//	RUN_TEST_CASE(UsartIO_DMA, OpenHalNotResetState);
//	RUN_TEST_CASE(UsartIO_DMA, OpenHalInitOkReceiveOk);
//	
	
	// args invalid
	// args valid, hal state not continuable
	// otherwise, whether (upper) buffer space has enough space or not
	// and whether hal tx state are busy (tx_busy & tx_rx_busy are busy, 
	// ready and rx_busy are not (tx) busy) are orthogonal (parallel) states.
	// no need to further consider HAL_UART_Transmit_DMA return values.
	// This function returns only HAL_OK when tx not busy and HAL_BUSY when tx busy.
	// (except invalid args, HAL_ERROR, we make sure this won't happen.)
	RUN_TEST_CASE(UsartIO_DMA, WriteInvalidArgs);
	RUN_TEST_CASE(UsartIO_DMA, WriteHalStateTimeoutErrorReset);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceAdequateAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceAdequateAndHalBusy);
//	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceInadequateAndHalReady);
//	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceInadequateAndHalBusy);
}


#include <string.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "errno_ex.h"
#include "uart_io.h"
#include "uart_io_private.h"



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

static 	UART_HandleTypeDef 			m_huart;

static	UART_IO_HandleTypeDef 	m_huio_default = 
{
	.handle = &m_huart,
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

static 	uint32_t 								m_rx_m0ar;
static 	int 										m_rx_ndtr;
static 	uint32_t								m_tx_m0ar;
static 	int											m_tx_ndtr;

extern 	HAL_StatusTypeDef __uart_io_rx_flip_buffer(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_ok(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_error(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_timeout(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
static 	HAL_StatusTypeDef rx_flip_buffer_mock_hal_busy(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);


/** utils begin **/
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
HAL_StatusTypeDef m_receive_hal_ok(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) 
{
	m_rx_m0ar = (uint32_t)pData;
	m_rx_ndtr = Size;
	
	return HAL_OK;
}

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


/** set and fill upper buffer **/
static void fill_rx_upper(int index, int offset, const char* src, int size)
{	
	/** upper buffer head will NEVER point to the first char in buffer **/
	TEST_ASSERT_TRUE(size + offset < UART_IO_BUFFER_SIZE);
	
	m_huio.rx_upper = m_huio.rbuf[index];
	m_huio.rx_head = &m_huio.rx_upper[offset];
	m_huio.rx_tail = m_huio.rx_head + size;
	
	if (size)
	{
		memmove(m_huio.rx_head, src, size);
	}
}

static void fill_tx_upper(int index, const char* src, int size)
{
	TEST_ASSERT_TRUE(size < UART_IO_BUFFER_SIZE);
	
	m_huio.tx_head = m_huio.tbuf[index];
	memmove(m_huio.tx_head, src, size);
	m_huio.tx_tail = m_huio.tx_head + size;
}

/** set and fill dma buffer **/
/** dst is set as dma buffer **/
static void fill_dma_buffer(char* dst, const char* src, int size)
{
	TEST_ASSERT_NOT_EQUAL(0, dst);
	TEST_ASSERT_NOT_EQUAL(0, src);
	TEST_ASSERT_NOT_EQUAL(0, size);
	TEST_ASSERT_TRUE(size < UART_IO_BUFFER_SIZE);
	
	m_rx_m0ar = (uint32_t)dst;
	m_rx_ndtr = UART_IO_BUFFER_SIZE - size;
	
	memmove(dst, src, size);
}

/** utils end **/

TEST_GROUP(UsartIO_DMA);

TEST_SETUP(UsartIO_DMA)
{
	memset(test_buffer, 0, TEST_BUFFER_SIZE);
	memset(&m_huio, 0, sizeof(m_huio));
	memset(&m_huart, 0, sizeof(m_huart));
	
	m_huart = m_huart_default;
	m_huio = m_huio_default;
	
	m_rx_m0ar = 0;
	m_rx_ndtr = 0;
	
	usart_apis = usart_apis_default;
}

TEST_TEAR_DOWN(UsartIO_DMA)
{
}

HAL_StatusTypeDef rx_flip_buffer_mock_hal_ok(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	if (m0ar) *m0ar = m_rx_m0ar;
	if (ndtr) *ndtr = m_rx_ndtr;
	
	m_rx_m0ar = (uint32_t)buf;
	m_rx_ndtr = size;
	
	return HAL_OK;
}

HAL_StatusTypeDef rx_flip_buffer_mock_hal_error(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_ERROR;
}

HAL_StatusTypeDef rx_flip_buffer_mock_hal_busy(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_BUSY;
}

HAL_StatusTypeDef rx_flip_buffer_mock_hal_timeout(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_TIMEOUT;
}

/******************************************************************************
 *
 * Read
 *
 * int UART_IO_Read(UART_IO_HandleTypeDef* h, char* buffer, size_t buffer_size)
 *
 *****************************************************************************/
TEST(UsartIO_DMA, ReadInvalidArgs)
{
	char c;
	
	TEST_ASSERT_EQUAL(-1, UART_IO_Read(0, &c, 1));					/** null handle **/
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(-1, UART_IO_Read(&m_huio, 0, 1));			/** null buf p	**/
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(0, UART_IO_Read(&m_huio, &c, 0));			/** no error checking as linux **/
}

TEST(UsartIO_DMA, ReadWhenBytesToReadLessThanOrEqualToBytesInBuffer)
{
	char sample[] = "test";
	char buf[64];
	int read;
	
	/** prepare **/
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, sample, strlen(sample));	/** use buffer 1 as upper buffer **/
	
	read = UART_IO_Read(&m_huio, buf, strlen(sample));
	
	TEST_ASSERT_EQUAL(strlen(sample), read);
	TEST_ASSERT_EQUAL_MEMORY(sample, buf, strlen(sample));
}


TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalReady)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;
	
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, soft, strlen(soft));
	fill_dma_buffer(m_huio.rbuf[0], hard, strlen(hard)); 
	usart_apis.UART_IO_RxFlipBuffer = rx_flip_buffer_mock_hal_ok;
	
	read = UART_IO_Read(&m_huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft) + strlen(hard), read);
	TEST_ASSERT_EQUAL_STRING("testdriven", buf);
}


TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalError)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;
	
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, soft, strlen(soft));
	fill_dma_buffer(m_huio.rbuf[0], hard, strlen(hard)); 
	usart_apis.UART_IO_RxFlipBuffer = rx_flip_buffer_mock_hal_error;
	
	read = UART_IO_Read(&m_huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);	
}

TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalBusy)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;
	
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, soft, strlen(soft));
	fill_dma_buffer(m_huio.rbuf[0], hard, strlen(hard)); 
	usart_apis.UART_IO_RxFlipBuffer = rx_flip_buffer_mock_hal_busy;
	
	read = UART_IO_Read(&m_huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);	
}


TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalTimeout)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;
	
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, soft, strlen(soft));
	fill_dma_buffer(m_huio.rbuf[0], hard, strlen(hard)); 
	usart_apis.UART_IO_RxFlipBuffer = rx_flip_buffer_mock_hal_timeout;
	
	read = UART_IO_Read(&m_huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);	
}


TEST(UsartIO_DMA, ReadWhenBufferEmptyAndHalError)
{
	char buf[64];
	int read;
	
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, 0, 0);	/** size 0 **/
	usart_apis.UART_IO_RxFlipBuffer = rx_flip_buffer_mock_hal_error;
	
	read = UART_IO_Read(&m_huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EINVAL, errno);	
}

TEST(UsartIO_DMA, ReadWhenBufferEmptyAndHalBusy)
{
	char buf[64];
	int read;
	
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, 0, 0);	/** size 0 **/
	usart_apis.UART_IO_RxFlipBuffer = rx_flip_buffer_mock_hal_busy;
	
	read = UART_IO_Read(&m_huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EBUSY, errno);	
}

TEST(UsartIO_DMA, ReadWhenBufferEmptyAndHalTimeout)
{
	char buf[64];
	int read;
	
	memset(buf, 0, sizeof(buf));
	fill_rx_upper(1, 1, 0, 0);	/** size 0 **/
	usart_apis.UART_IO_RxFlipBuffer = rx_flip_buffer_mock_hal_timeout;
	
	read = UART_IO_Read(&m_huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EIO, errno);	
}

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

/******************************************************************************
 *
 * write
 *
 * int UART_IO_Write(UART_IO_HandleTypeDef* handle, char* buffer, size_t size);
 *
 *****************************************************************************/
TEST(UsartIO_DMA, WriteInvalidArgs)
{
	char c;
	
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(0, &c, 1));
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&m_huio, 0, 1));
	TEST_ASSERT_EQUAL(EINVAL, errno);
	
	/** no effect for errno**/
	/** may be changed future, man page write **/
	TEST_ASSERT_EQUAL(0, UART_IO_Write(&m_huio, &c, 0)); 
}

TEST(UsartIO_DMA, WriteHalStateTimeout)
{
	char c;

	errno = 0;
	m_huio.handle->State = HAL_UART_STATE_TIMEOUT;
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&m_huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);
	
	errno = 0;
	m_huio.handle->State = HAL_UART_STATE_RESET;
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&m_huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);

	errno = 0;
	m_huio.handle->State = HAL_UART_STATE_ERROR;
	TEST_ASSERT_EQUAL(-1, UART_IO_Write(&m_huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);
	
}

TEST(UsartIO_DMA, WriteBufferSpaceAdequateAndHalReady)
{
	char s1[] = "test";
	char s2[] = "driven";
	char s3[] = "testdriven";
	int written;
	
	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_ok;
	fill_tx_upper(1, s1, strlen(s1));
	m_huio.handle->State = HAL_UART_STATE_READY;	/** otherwise trapped **/
	
	written = UART_IO_Write(&m_huio, s2, strlen(s2));

	TEST_ASSERT_EQUAL(strlen(s2), written);
	
	TEST_ASSERT_EQUAL_MEMORY(s3, tx_dma_buffer, strlen(s3));
	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], (char*)m_tx_m0ar);
	TEST_ASSERT_EQUAL(strlen(s3), m_tx_ndtr);
	
	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[0], m_huio.tx_head);
	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[0], m_huio.tx_tail);
}

TEST(UsartIO_DMA, WriteBufferSpaceAdequateAndHalBusy)
{
	char s1[] = "test";
	char s2[] = "driven";
	char s3[] = "testdriven";
	int written;
	
	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_busy;
	fill_tx_upper(1, s1, strlen(s1));
	m_huio.handle->State = HAL_UART_STATE_READY;	/** otherwise trapped **/
	
	written = UART_IO_Write(&m_huio, s2, strlen(s2));

	TEST_ASSERT_EQUAL(strlen(s2), written);
	
	TEST_ASSERT_EQUAL_MEMORY(s3, m_huio.tbuf[1], strlen(s3));
	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], m_huio.tx_head);
	TEST_ASSERT_EQUAL(strlen(s3), m_huio.tx_tail - m_huio.tx_head);
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
	m_huio.handle->State = HAL_UART_STATE_READY;	/** otherwise trapped **/
	
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
	m_huio.handle->State = HAL_UART_STATE_READY;	/** otherwise trapped **/
	
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
	RUN_TEST_CASE(UsartIO_DMA, ReadInvalidArgs);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadLessThanOrEqualToBytesInBuffer);
	
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalError);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalBusy);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalTimeout);
	
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBufferEmptyAndHalError);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBufferEmptyAndHalBusy);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBufferEmptyAndHalTimeout);
	
//	RUN_TEST_CASE(UsartIO_DMA, OpenInvalidArgs);
//	RUN_TEST_CASE(UsartIO_DMA, OpenPortUnavailable);
//	RUN_TEST_CASE(UsartIO_DMA, OpenHalNotResetState);
//	RUN_TEST_CASE(UsartIO_DMA, OpenHalInitOkReceiveOk);
//	
	RUN_TEST_CASE(UsartIO_DMA, WriteInvalidArgs);
	RUN_TEST_CASE(UsartIO_DMA, WriteHalStateTimeout);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceAdequateAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceAdequateAndHalBusy);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceInadequateAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceInadequateAndHalBusy);
}

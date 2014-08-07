#include <string.h>
#include "errno_ex.h"
#include "msp.h"


static void* __testdata = 0;

void set_testdata(void* testdata)
{
	__testdata = testdata;
}

void* get_testdata(void)
{
	return __testdata;
}

struct create_uartex_handle_testdata
{
	const UARTEX_ConfigTypeDef* config;
	struct msp_factory * msp;
	
	int gpioex_init_fail_countdown;
	int gpioex_init_mock_called;
	
	int create_dmaex_handle_fail_countdown;
	int create_dmaex_handle_mock_called;
	
	int irq_handle_init_by_config_fail_countdown;
	int irq_handle_init_by_config_called;
	
	int uartex_handle_init_by_config_fail_countdown;
	int uartex_handle_init_by_config_called;
};

TEST_GROUP(MSP);

TEST_SETUP(MSP)
{

	struct create_uartex_handle_testdata * td = 
		(struct create_uartex_handle_testdata * )get_testdata();
	
	
	/** reset internal call time statistics **/
	td->gpioex_init_mock_called = 0;
	td->create_dmaex_handle_mock_called = 0;
	td->irq_handle_init_by_config_called = 0;
	td->uartex_handle_init_by_config_called = 0;
	
	/** set/restore operators **/
	td->msp->create_dmaex_handle = msp_create_dmaex_handle;
	td->msp->gpioex_init_by_config = GPIOEX_InitByConfig;
	td->msp->irq_handle_init_by_config = IRQ_Handle_InitByConfig;
	td->msp->uartex_handle_init_by_config = UARTEX_Handle_InitByConfig;
}
	
TEST_TEAR_DOWN(MSP)
{}

static UARTEX_HandleTypeDef* mock_create_uartex_handle(struct msp_factory* msp, const UARTEX_ConfigTypeDef* cfg)		
{
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();	
	
	TEST_ASSERT_NOT_NULL(testdata);
	TEST_ASSERT_NOT_NULL(testdata->msp);
	
	TEST_ASSERT_EQUAL_HEX32(testdata->msp, msp);
	TEST_ASSERT_EQUAL_HEX32(testdata->msp->board_config->uart2, cfg);
	
	return (UARTEX_HandleTypeDef*)(0xDEADBEEF);
};

TEST(MSP, CreateUARTEXHandleByPortInvalidArgs)
{
	UARTEX_HandleTypeDef* h;
	struct msp_factory msp;

	errno = 0;
	h = msp_create_uartex_handle_by_port(NULL, 2);
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EINVAL, errno);
	
	msp.board_config = 0;
	
	errno = 0;
	h = msp_create_uartex_handle_by_port(&msp, 2);
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EINVAL, errno);
	
	msp.board_config = (Board_ConfigTypeDef*)0xDEADBEEF;
	
	errno = 0;
	h = msp_create_uartex_handle_by_port(&msp, 0);
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EINVAL, errno);
	
	errno = 0;
	h = msp_create_uartex_handle_by_port(&msp, 7);
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EINVAL, errno);	
}

TEST(MSP, CreateUARTEXHandleByPortSuccess)
{
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();	
	
	testdata->msp->create_uartex_handle = mock_create_uartex_handle;
	
	UARTEX_HandleTypeDef* h = msp_create_uartex_handle_by_port(testdata->msp, 2);
	TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, h);
}

///////////////////////////////////////////////////////////////////////////////
// TEST CASES for UARTEX Handle factory methods
static int mock_gpioex_init_by_config(GPIOEX_TypeDef* gpioex, const GPIO_ConfigTypeDef* config, GPIO_ClockProviderTypeDef* clk)
{
	struct create_uartex_handle_testdata* td = (struct create_uartex_handle_testdata *)get_testdata();
	
	td->gpioex_init_mock_called++;
	
	if (td->gpioex_init_fail_countdown == 0) {
		return -EINVAL;
	}
	
	td->gpioex_init_fail_countdown--;
	
	memset(gpioex, 0, sizeof(*gpioex));
	return 0;
}

static DMAEX_HandleTypeDef*	mock_create_dmaex_handle(struct msp_factory * msp, const DMA_ConfigTypeDef * dmacfg, const IRQ_ConfigTypeDef * irqcfg)
{
	/** assuming this is malloced handle **/
	TEST_ASSERT_NOT_NULL(msp);
	
	struct create_uartex_handle_testdata* td = (struct create_uartex_handle_testdata *)get_testdata();
	td->create_dmaex_handle_mock_called++;
	
	if (td->create_dmaex_handle_fail_countdown == 0) {
		errno = EMAGIC;	// see comment in test case
		return NULL;
	}
	
	td->create_dmaex_handle_fail_countdown--;
	
	//// fall-back
	return msp_create_dmaex_handle(msp, dmacfg, irqcfg);
}

static int	mock_irq_handle_init_by_config(IRQ_HandleTypeDef* h, const IRQ_ConfigTypeDef* config, IRQ_HandleRegistryTypeDef* registry)
{
	struct create_uartex_handle_testdata* td = (struct create_uartex_handle_testdata *)get_testdata();
	
	td->irq_handle_init_by_config_called++;
	
	if (td->irq_handle_init_by_config_fail_countdown == 0) {
		return -EMAGIC;
	}
	
	td->irq_handle_init_by_config_fail_countdown--;
	return 0;
}

static int mock_uartex_handle_init_by_config(UARTEX_HandleTypeDef* h, const UART_ConfigTypeDef	*config, GPIOEX_TypeDef	*rxpin, GPIOEX_TypeDef *txpin, 
		DMAEX_HandleTypeDef *hdmaex_rx, DMAEX_HandleTypeDef *hdmaex_tx, IRQ_HandleTypeDef *hirq, const struct UARTEX_Operations	*ops) {
	
	struct create_uartex_handle_testdata* td = (struct create_uartex_handle_testdata *)get_testdata();
			
	td->uartex_handle_init_by_config_called++;
			
	if (td->uartex_handle_init_by_config_fail_countdown == 0) {
		return -EMAGIC;
	}
	
	td->irq_handle_init_by_config_fail_countdown--;
	return 0;
}

TEST(MSP, CreateUARTEXHandleGPIOEXInitFailFirstCall)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	testdata->gpioex_init_fail_countdown = 0;
	testdata->gpioex_init_mock_called = 0;
	testdata->msp->gpioex_init_by_config = mock_gpioex_init_by_config;
	errno = 0;
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);	
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(1, testdata->gpioex_init_mock_called);
}

TEST(MSP, CreateUARTEXHandleGPIOEXInitFailSecondCall)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	testdata->gpioex_init_fail_countdown = 1;
	testdata->gpioex_init_mock_called = 0;
	testdata->msp->gpioex_init_by_config = mock_gpioex_init_by_config;
	errno = 0;

	h = msp_create_uartex_handle(testdata->msp, testdata->config);	
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(2, testdata->gpioex_init_mock_called);
}

TEST(MSP, CreateUARTEXHandleCreateDMAEXFailFirstCall)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	testdata->create_dmaex_handle_fail_countdown = 0;
	testdata->msp->create_dmaex_handle = mock_create_dmaex_handle;
	errno = 0;	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);	
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EMAGIC, errno); /** This errno is actually set by 'real' func, and varies in different situation.
																				hence we don't need to test it, or we test it with magic number to
																				assure it is set by (mock) func and not touched further. **/
	
	TEST_ASSERT_EQUAL(1, testdata->create_dmaex_handle_mock_called);
}

TEST(MSP, CreateUARTEXHandleCreateDMAEXFailSecondCall)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	testdata->create_dmaex_handle_fail_countdown = 1;
	testdata->msp->create_dmaex_handle = mock_create_dmaex_handle;
	errno = 0;	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);	
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EMAGIC, errno); /** This errno is actually set by 'real' func, and varies in different situation.
																				hence we don't need to test it, or we test it with magic number to
																				assure it is set by (mock) func and not touched further. **/
	
	TEST_ASSERT_EQUAL(2, testdata->create_dmaex_handle_mock_called);
}

TEST(MSP, CreateUARTEXHandleIRQHandleInitFail)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	testdata->irq_handle_init_by_config_fail_countdown = 0;
	testdata->msp->irq_handle_init_by_config = mock_irq_handle_init_by_config;
	errno = 0;	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);	
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EMAGIC, errno);
	TEST_ASSERT_EQUAL(1, testdata->irq_handle_init_by_config_called);
}

TEST(MSP, CreateUARTEXHandleUARTEXHandleInitFail)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	testdata->uartex_handle_init_by_config_fail_countdown = 0;
	testdata->msp->uartex_handle_init_by_config = mock_uartex_handle_init_by_config;
	errno = 0;	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);	
	
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(EMAGIC, errno);
	TEST_ASSERT_EQUAL(1, testdata->uartex_handle_init_by_config_called);
}

TEST(MSP, CreateUARTEXHandleMallocFail)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(0);	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);
	
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(1);	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);	
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(2);	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);	
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(3);	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);	
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(4);	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);	
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(5);	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);	
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(6);	
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);	
	
	errno = 0;
	UnityMalloc_MakeMallocFailAfterCount(7);	// no more, 7 times of successful allocation still fail.
																						// actually the func needs 8 times of malloc success.	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	TEST_ASSERT_NULL(h);
	TEST_ASSERT_EQUAL(ENOMEM, errno);	
}

TEST(MSP, CreateUARTEXHandleSuccess)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
		
	TEST_ASSERT_NOT_NULL(h);
		
	TEST_ASSERT_NOT_NULL(h->hdmaex_rx);
	if (h->hdmaex_rx)
	{
		TEST_ASSERT_EQUAL_HEX32(testdata->config->dmarx->Instance, h->hdmaex_rx->hdma.Instance);
		TEST_ASSERT_EQUAL(testdata->config->dmarx_irq->irqn, h->hdmaex_rx->hirq->irqn);
	}
	
	TEST_ASSERT_NOT_NULL(h->hdmaex_tx);
	if (h->hdmaex_tx)
	{
		TEST_ASSERT_EQUAL_HEX32(testdata->config->dmatx->Instance, h->hdmaex_tx->hdma.Instance);
		TEST_ASSERT_EQUAL(testdata->config->dmatx_irq->irqn, h->hdmaex_tx->hirq->irqn);
	}
	
	TEST_ASSERT_NOT_NULL(h->rxpin);
	
	if (h->rxpin)
	{
		TEST_ASSERT_EQUAL(testdata->config->rxpin->instance, h->rxpin->instance);
		TEST_ASSERT_EQUAL(testdata->config->rxpin->init.Pin, h->rxpin->init.Pin);
	}
	
	TEST_ASSERT_NOT_NULL(h->txpin);
	if (h->txpin)
	{
		TEST_ASSERT_EQUAL(testdata->config->txpin->instance, h->txpin->instance);
		TEST_ASSERT_EQUAL(testdata->config->txpin->init.Pin, h->txpin->init.Pin);
	}
	
	TEST_ASSERT_NOT_NULL(h->hirq);
	if (h->hirq)
	{
		TEST_ASSERT_EQUAL(testdata->config->uart_irq->irqn, h->hirq->irqn);
	}

	TEST_ASSERT_EQUAL_MEMORY(&UARTEX_Ops_DefaultConfig, &h->ops, sizeof(h->ops));
	
	TEST_ASSERT_EQUAL(testdata->config->uart->Instance, h->huart.Instance);

	testdata->msp->destroy_dmaex_handle(testdata->msp, h->hdmaex_rx);
	testdata->msp->destroy_dmaex_handle(testdata->msp, h->hdmaex_tx);

	free(h->rxpin);
	free(h->txpin);
	free(h->hirq);
	
	free(h);
}


TEST(MSP, DestroyUARTEXHandleFull)
{
	UARTEX_HandleTypeDef* h;
	
	struct create_uartex_handle_testdata * testdata = 
		(struct create_uartex_handle_testdata *)get_testdata();
	
	h = msp_create_uartex_handle(testdata->msp, testdata->config);
	msp_destroy_uartex_handle(testdata->msp, h);
}
///////////////////////////////////////////////////////////////////////////////
// DMA Handle Factory Methods
TEST(MSP, CreateDMAEXHandle)
{
	GPIO_ClockProviderTypeDef			gpio_clk;
	DMA_ClockProviderTypeDef			dma_clk;
	IRQ_HandleRegistryTypeDef			registry;
	DMA_ConfigTypeDef							dma_config;
	IRQ_ConfigTypeDef							irq_config;
	struct msp_factory						msp;

	DMAEX_HandleTypeDef*					h;
	
	memset(&dma_config, 0xA5, sizeof(dma_config));
	memset(&irq_config, 0xB5, sizeof(irq_config));
	irq_config.irqn = USART2_IRQn;			/** must be something valid **/
	dma_config.Instance = DMA1_Stream5; /** must be something valid **/
	
	memset(&msp, 0, sizeof(msp));
	msp.dma_clk = &dma_clk;
	msp.gpio_clk = &gpio_clk;
	msp.irq_registry = &registry;
	
	
	// h = DMAEX_Handle_FactoryCreate(&clk, &registry, &dma_config, &irq_config);
	/** DMAEX_HandleTypeDef*	msp_create_dmaex_handle(struct msp_factory * msp, 
	const DMA_ConfigTypeDef dmacfg, const IRQ_ConfigTypeDef irqcfg); **/
	h = msp_create_dmaex_handle(&msp, &dma_config, &irq_config);
	
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL_HEX32(&dma_clk, h->clk);
	
	TEST_ASSERT_EQUAL_HEX32(dma_config.Instance, h->hdma.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&dma_config.Init, &h->hdma.Init, sizeof(DMA_InitTypeDef));

	TEST_ASSERT_EQUAL(irq_config.irqn, h->hirq->irqn);
	TEST_ASSERT_EQUAL(irq_config.preempt_priority, h->hirq->preempt_priority);
	TEST_ASSERT_EQUAL(irq_config.sub_priority, h->hirq->sub_priority);
	TEST_ASSERT_EQUAL(&registry, h->hirq->registry);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->hirq->state);
	
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, h->state);
	
	if (h->hirq) free(h->hirq);
	if (h) free(h);
}

TEST(MSP, DestroyDMAEXHandle)
{
	GPIO_ClockProviderTypeDef			gpio_clk;
	DMA_ClockProviderTypeDef			dma_clk;
	IRQ_HandleRegistryTypeDef			registry;
	DMA_ConfigTypeDef							dma_config;
	IRQ_ConfigTypeDef							irq_config;
	struct msp_factory						msp;

	DMAEX_HandleTypeDef*					h;
	
	memset(&dma_config, 0xA5, sizeof(dma_config));
	memset(&irq_config, 0xB5, sizeof(irq_config));
	irq_config.irqn = USART2_IRQn;			/** must be something valid **/
	dma_config.Instance = DMA1_Stream5; /** must be something valid **/
	
	memset(&msp, 0, sizeof(msp));
	msp.dma_clk = &dma_clk;
	msp.gpio_clk = &gpio_clk;
	msp.irq_registry = &registry;
	
	h = msp_create_dmaex_handle(&msp, &dma_config, &irq_config);
	TEST_ASSERT_NOT_NULL(h);
	
	msp_destroy_dmaex_handle(&msp, h);
}

TEST_GROUP_RUNNER(MSP)
{
	/////////////////////////////////////////////////////////////////////////////
	// prepare global test data
	const UARTEX_ConfigTypeDef cfg =
	{
		.uart = &UART2_DefaultConfig,
		.rxpin = &PD6_As_Uart2Rx_DefaultConfig,
		.txpin = &PD5_As_Uart2Tx_DefaultConfig,
		.dmarx = &DMA_Uart2Rx_DefaultConfig,
		.dmarx_irq = &IRQ_Uart2RxDMA_DefaultConfig,
		.dmatx = &DMA_Uart2Tx_DefaultConfig,
		.dmatx_irq = &IRQ_Uart2TxDMA_DefaultConfig,
		.uart_irq = &IRQ_Uart2_DefaultConfig,
		.uartex_ops = &UARTEX_Ops_DefaultConfig,
	};
	
	Board_ConfigTypeDef board = { .uart2 = &cfg, };
	
	GPIO_ClockProviderTypeDef gpio_clk;
	DMA_ClockProviderTypeDef dma_clk;
	IRQ_HandleRegistryTypeDef irq_registry;
	
	struct msp_factory msp = {
		
		.board_config = &board,
		
		.gpio_clk = &gpio_clk,
		.dma_clk = &dma_clk,
		.irq_registry = &irq_registry,
		
		.create_uartex_handle_by_port = msp_create_uartex_handle_by_port,
		.create_uartex_handle = msp_create_uartex_handle,
		.destroy_uartex_handle = msp_destroy_uartex_handle,
		
		.create_dmaex_handle = msp_create_dmaex_handle,
		.destroy_dmaex_handle = msp_destroy_dmaex_handle,
		
		.gpioex_init_by_config = GPIOEX_InitByConfig,
		.irq_handle_init_by_config = IRQ_Handle_InitByConfig,
	};	

	struct create_uartex_handle_testdata testdata = {
		.config = &cfg,
		.msp = &msp,
	};
	
	set_testdata(&testdata);
	
	/////////////////////////////////////////////////////////////////////////////
	
	// RUN_TEST_CASE(MSP, CreateHuartEx);
	RUN_TEST_CASE(MSP, CreateDMAEXHandle);
	RUN_TEST_CASE(MSP, DestroyDMAEXHandle);

	
	/////////////////////////////////////////////////////////////////////////////
	// use global testdata
	RUN_TEST_CASE(MSP, CreateUARTEXHandleGPIOEXInitFailFirstCall);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleGPIOEXInitFailSecondCall);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleCreateDMAEXFailFirstCall);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleCreateDMAEXFailSecondCall);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleIRQHandleInitFail);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleUARTEXHandleInitFail);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleMallocFail);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleSuccess);
	RUN_TEST_CASE(MSP, DestroyUARTEXHandleFull);
	
	
	RUN_TEST_CASE(MSP, CreateUARTEXHandleByPortInvalidArgs);
	RUN_TEST_CASE(MSP, CreateUARTEXHandleByPortSuccess);
	/////////////////////////////////////////////////////////////////////////////
	
	set_testdata(NULL);
}




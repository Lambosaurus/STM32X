
#include "USB_PCD.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#elif defined(STM32F0)

#endif

#define USB_ENDPOINTS		8
#define PMA_SIZE			512
#define BTABLE_SIZE			(USB_ENDPOINTS * 8)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

// DELETE THIS
USBD_HandleTypeDef hUsbDeviceFS;
PCD_HandleTypeDef * hpcd = &hpcd_USB_FS;

static struct {
	uint16_t pma_head;
}gPCD;

/*
 * PUBLIC FUNCTIONS
 */

void USB_PCD_Init(void)
{
	gPCD.pma_head = 0;

	hpcd_USB_FS.pData = &hUsbDeviceFS;
	hpcd_USB_FS.Instance = USB;

	for (uint32_t i = 0U; i < USB_ENDPOINTS; i++)
	{
		hpcd->IN_ep[i].is_in = 1U;
		hpcd->IN_ep[i].num = i;
		hpcd->IN_ep[i].type = EP_TYPE_CTRL;
		hpcd->IN_ep[i].maxpacket = 0U;
		hpcd->IN_ep[i].xfer_buff = 0U;
		hpcd->IN_ep[i].xfer_len = 0U;
	}
	for (uint32_t i = 0U; i < USB_ENDPOINTS; i++)
	{
		hpcd->OUT_ep[i].is_in = 0U;
		hpcd->OUT_ep[i].num = i;
		hpcd->OUT_ep[i].type = EP_TYPE_CTRL;
		hpcd->OUT_ep[i].maxpacket = 0U;
		hpcd->OUT_ep[i].xfer_buff = 0U;
		hpcd->OUT_ep[i].xfer_len = 0U;
	}

	USB->CNTR = USB_CNTR_FRES; // Issue reset
	USB->CNTR = 0U;
	USB->ISTR = 0U;
	USB->BTABLE = BTABLE_ADDRESS;
	USB_PCD_AllocPMA(BTABLE_SIZE);

	hpcd->USB_Address = 0U;
	hpcd->State = HAL_PCD_STATE_READY;

#ifdef USB_USE_LPM
	hpcd->LPM_State = LPM_L0;
	USB->LPMCSR |= USB_LPMCSR_LMPEN;
	USB->LPMCSR |= USB_LPMCSR_LPMACK;
#endif
}

void USB_PCD_Start(void)
{
	// Enable interrupt sources
	USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM
			  | USB_CNTR_WKUPM | USB_CNTR_SUSPM
            // | USB_CNTR_SOFM | USB_CNTR_ESOFM | USB_CNTR_ERRM
              | USB_CNTR_L1REQM;
			// USB_CNTR_RESUME remote wakeup mode?
	// Enable DP pullups
	USB->BCDR |= USB_BCDR_DPPU;
}

void USB_PCD_Stop(void)
{
	// disable all interrupts and force USB reset
	USB->CNTR = USB_CNTR_FRES;
	// clear interrupt status register
	USB->ISTR = 0U;
	// switch-off device
	USB->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN;
	// Disable DP pullups
	USB->BCDR &= ~USB_BCDR_DPPU;

	// DELETE THIS.
	hpcd_USB_FS.State = HAL_PCD_STATE_RESET;
}

void USB_PCD_EP_Open(uint8_t endpoint, uint8_t type, uint16_t size, bool doublebuffer)
{
	PCD_EPTypeDef *ep;
	if (endpoint & 0x80U)
	{
		ep = &hpcd->IN_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_in[endpoint & EP_ADDR_MSK].is_used = 1;
	}
	else
	{
		ep = &hpcd->OUT_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_out[endpoint & EP_ADDR_MSK].is_used = 1;
	}
	ep->maxpacket = size;
	ep->type = type;
	if (doublebuffer)
	{
		ep->doublebuffer = 1;
		ep->pmaaddr0 = USB_PCD_AllocPMA(size);
		ep->pmaaddr1 = USB_PCD_AllocPMA(size);
	}
	else
	{
		ep->pmaadress = USB_PCD_AllocPMA(size);
	}
	USB_ActivateEndpoint(USB, ep);
}

void USB_PCD_EP_Close(uint8_t endpoint)
{
	PCD_EPTypeDef *ep;
	if (endpoint & 0x80U)
	{
		ep = &hpcd->IN_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_in[endpoint & EP_ADDR_MSK].is_used = 0;
	}
	else
	{
		ep = &hpcd->OUT_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_out[endpoint & EP_ADDR_MSK].is_used = 0;
	}
	USB_DeactivateEndpoint(USB, ep);
}

void USB_PCD_EP_StartRx(uint8_t endpoint, uint8_t * data, uint32_t count)
{
	PCD_EPTypeDef * ep = &(hpcd_USB_FS.OUT_ep[endpoint & EP_ADDR_MSK]);
	ep->xfer_buff = data;
	ep->xfer_len = count;
	ep->xfer_count = 0;
	USB_EPStartXfer(USB, ep);
}

void USB_PCD_EP_StartTx(uint8_t endpoint, const uint8_t * data, uint32_t count)
{
	PCD_EPTypeDef * ep = &(hpcd_USB_FS.IN_ep[endpoint & EP_ADDR_MSK]);
	ep->xfer_buff = (uint8_t *)data;
	ep->xfer_len = count;
	ep->xfer_fill_db = 1;
	ep->xfer_len_db = count;
	ep->xfer_count = 0;
	hUsbDeviceFS.ep_in[endpoint & EP_ADDR_MSK].total_length = count;
	USB_EPStartXfer(USB, ep);
}

uint16_t USB_PCD_AllocPMA(uint16_t size)
{
	uint16_t head = gPCD.pma_head;
	gPCD.pma_head += size;
	if (gPCD.pma_head > PMA_SIZE)
	{
		// TODO: fix this?
		__BKPT();
	}
	return head;
}

void USB_PCD_SetAddress(uint8_t address)
{
	hpcd->USB_Address = address;

	if (address == 0U)
	{
		/* set device address and enable function */
		USB->DADDR = (uint16_t)USB_DADDR_EF;
	}
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */



static uint16_t HAL_PCD_EP_DB_Receive(PCD_HandleTypeDef *hpcd, PCD_EPTypeDef *ep, uint16_t wEPVal);
static HAL_StatusTypeDef HAL_PCD_EP_DB_Transmit(PCD_HandleTypeDef *hpcd, PCD_EPTypeDef *ep, uint16_t wEPVal);

HAL_StatusTypeDef USB_PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd)
{
  PCD_EPTypeDef *ep;
  uint16_t count, wIstr, wEPVal, TxByteNbre;
  uint8_t epindex;

  /* stay in loop while pending interrupts */
  while ((hpcd->Instance->ISTR & USB_ISTR_CTR) != 0U)
  {
    wIstr = hpcd->Instance->ISTR;

    /* extract highest priority endpoint number */
    epindex = (uint8_t)(wIstr & USB_ISTR_EP_ID);

    if (epindex == 0U)
    {
      /* Decode and service control endpoint interrupt */

      /* DIR bit = origin of the interrupt */
      if ((wIstr & USB_ISTR_DIR) == 0U)
      {
        /* DIR = 0 */

        /* DIR = 0 => IN  int */
        /* DIR = 0 implies that (EP_CTR_TX = 1) always */
        PCD_CLEAR_TX_EP_CTR(hpcd->Instance, PCD_ENDP0);
        ep = &hpcd->IN_ep[0];

        ep->xfer_count = PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num);
        ep->xfer_buff += ep->xfer_count;

        /* TX COMPLETE */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
        hpcd->DataInStageCallback(hpcd, 0U);
#else
        HAL_PCD_DataInStageCallback(hpcd, 0U);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */

        if ((hpcd->USB_Address > 0U) && (ep->xfer_len == 0U))
        {
          hpcd->Instance->DADDR = ((uint16_t)hpcd->USB_Address | USB_DADDR_EF);
          hpcd->USB_Address = 0U;
        }
      }
      else
      {
        /* DIR = 1 */

        /* DIR = 1 & CTR_RX => SETUP or OUT int */
        /* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */
        ep = &hpcd->OUT_ep[0];
        wEPVal = PCD_GET_ENDPOINT(hpcd->Instance, PCD_ENDP0);

        if ((wEPVal & USB_EP_SETUP) != 0U)
        {
          /* Get SETUP Packet */
          ep->xfer_count = PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);

          USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup,
                      ep->pmaadress, (uint16_t)ep->xfer_count);

          /* SETUP bit kept frozen while CTR_RX = 1 */
          PCD_CLEAR_RX_EP_CTR(hpcd->Instance, PCD_ENDP0);

          /* Process SETUP Packet*/
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
          hpcd->SetupStageCallback(hpcd);
#else
          HAL_PCD_SetupStageCallback(hpcd);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
        }
        else if ((wEPVal & USB_EP_CTR_RX) != 0U)
        {
          PCD_CLEAR_RX_EP_CTR(hpcd->Instance, PCD_ENDP0);

          /* Get Control Data OUT Packet */
          ep->xfer_count = PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);

          if ((ep->xfer_count != 0U) && (ep->xfer_buff != 0U))
          {
            USB_ReadPMA(hpcd->Instance, ep->xfer_buff,
                        ep->pmaadress, (uint16_t)ep->xfer_count);

            ep->xfer_buff += ep->xfer_count;

            /* Process Control Data OUT Packet */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
            hpcd->DataOutStageCallback(hpcd, 0U);
#else
            HAL_PCD_DataOutStageCallback(hpcd, 0U);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
          }

          PCD_SET_EP_RX_CNT(hpcd->Instance, PCD_ENDP0, ep->maxpacket);
          PCD_SET_EP_RX_STATUS(hpcd->Instance, PCD_ENDP0, USB_EP_RX_VALID);
        }
      }
    }
    else
    {
      /* Decode and service non control endpoints interrupt */
      /* process related endpoint register */
      wEPVal = PCD_GET_ENDPOINT(hpcd->Instance, epindex);

      if ((wEPVal & USB_EP_CTR_RX) != 0U)
      {
        /* clear int flag */
        PCD_CLEAR_RX_EP_CTR(hpcd->Instance, epindex);
        ep = &hpcd->OUT_ep[epindex];

        /* OUT Single Buffering */
        if (ep->doublebuffer == 0U)
        {
          count = (uint16_t)PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);

          if (count != 0U)
          {
            USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaadress, count);
          }
        }
        else
        {
          /* manage double buffer bulk out */
          if (ep->type == EP_TYPE_BULK)
          {
            count = HAL_PCD_EP_DB_Receive(hpcd, ep, wEPVal);
          }
          else /* manage double buffer iso out */
          {
            /* free EP OUT Buffer */
            PCD_FreeUserBuffer(hpcd->Instance, ep->num, 0U);

            if ((PCD_GET_ENDPOINT(hpcd->Instance, ep->num) & USB_EP_DTOG_RX) != 0U)
            {
              /* read from endpoint BUF0Addr buffer */
              count = (uint16_t)PCD_GET_EP_DBUF0_CNT(hpcd->Instance, ep->num);

              if (count != 0U)
              {
                USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaaddr0, count);
              }
            }
            else
            {
              /* read from endpoint BUF1Addr buffer */
              count = (uint16_t)PCD_GET_EP_DBUF1_CNT(hpcd->Instance, ep->num);

              if (count != 0U)
              {
                USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaaddr1, count);
              }
            }
          }
        }
        /* multi-packet on the NON control OUT endpoint */
        ep->xfer_count += count;
        ep->xfer_buff += count;

        if ((ep->xfer_len == 0U) || (count < ep->maxpacket))
        {
          /* RX COMPLETE */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
          hpcd->DataOutStageCallback(hpcd, ep->num);
#else
          HAL_PCD_DataOutStageCallback(hpcd, ep->num);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
        }
        else
        {
          (void) USB_EPStartXfer(hpcd->Instance, ep);
        }

      }

      if ((wEPVal & USB_EP_CTR_TX) != 0U)
      {
        ep = &hpcd->IN_ep[epindex];

        /* clear int flag */
        PCD_CLEAR_TX_EP_CTR(hpcd->Instance, epindex);

        /* Manage all non bulk transaction or Bulk Single Buffer Transaction */
        if ((ep->type != EP_TYPE_BULK) ||
            ((ep->type == EP_TYPE_BULK) && ((wEPVal & USB_EP_KIND) == 0U)))
        {
          /* multi-packet on the NON control IN endpoint */
          TxByteNbre = (uint16_t)PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num);

          if (ep->xfer_len > TxByteNbre)
          {
            ep->xfer_len -= TxByteNbre;
          }
          else
          {
            ep->xfer_len = 0U;
          }

          /* Zero Length Packet? */
          if (ep->xfer_len == 0U)
          {
            /* TX COMPLETE */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
            hpcd->DataInStageCallback(hpcd, ep->num);
#else
            HAL_PCD_DataInStageCallback(hpcd, ep->num);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
          }
          else
          {
            /* Transfer is not yet Done */
            ep->xfer_buff += TxByteNbre;
            ep->xfer_count += TxByteNbre;
            (void)USB_EPStartXfer(hpcd->Instance, ep);
          }
        }
        /* bulk in double buffer enable in case of transferLen> Ep_Mps */
        else
        {
          (void)HAL_PCD_EP_DB_Transmit(hpcd, ep, wEPVal);
        }
      }
    }
  }

  return HAL_OK;
}



static uint16_t HAL_PCD_EP_DB_Receive(PCD_HandleTypeDef *hpcd,
                                      PCD_EPTypeDef *ep, uint16_t wEPVal)
{
  uint16_t count;

  /* Manage Buffer0 OUT */
  if ((wEPVal & USB_EP_DTOG_RX) != 0U)
  {
    /* Get count of received Data on buffer0 */
    count = (uint16_t)PCD_GET_EP_DBUF0_CNT(hpcd->Instance, ep->num);

    if (ep->xfer_len >= count)
    {
      ep->xfer_len -= count;
    }
    else
    {
      ep->xfer_len = 0U;
    }

    if (ep->xfer_len == 0U)
    {
      /* set NAK to OUT endpoint since double buffer is enabled */
      PCD_SET_EP_RX_STATUS(hpcd->Instance, ep->num, USB_EP_RX_NAK);
    }

    /* Check if Buffer1 is in blocked sate which requires to toggle */
    if ((wEPVal & USB_EP_DTOG_TX) != 0U)
    {
      PCD_FreeUserBuffer(hpcd->Instance, ep->num, 0U);
    }

    if (count != 0U)
    {
      USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaaddr0, count);
    }
  }
  /* Manage Buffer 1 DTOG_RX=0 */
  else
  {
    /* Get count of received data */
    count = (uint16_t)PCD_GET_EP_DBUF1_CNT(hpcd->Instance, ep->num);

    if (ep->xfer_len >= count)
    {
      ep->xfer_len -= count;
    }
    else
    {
      ep->xfer_len = 0U;
    }

    if (ep->xfer_len == 0U)
    {
      /* set NAK on the current endpoint */
      PCD_SET_EP_RX_STATUS(hpcd->Instance, ep->num, USB_EP_RX_NAK);
    }

    /*Need to FreeUser Buffer*/
    if ((wEPVal & USB_EP_DTOG_TX) == 0U)
    {
      PCD_FreeUserBuffer(hpcd->Instance, ep->num, 0U);
    }

    if (count != 0U)
    {
      USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaaddr1, count);
    }
  }

  return count;
}


static HAL_StatusTypeDef HAL_PCD_EP_DB_Transmit(PCD_HandleTypeDef *hpcd,
                                                PCD_EPTypeDef *ep, uint16_t wEPVal)
{
  uint32_t len;
  uint16_t TxByteNbre;

  /* Data Buffer0 ACK received */
  if ((wEPVal & USB_EP_DTOG_TX) != 0U)
  {
    /* multi-packet on the NON control IN endpoint */
    TxByteNbre = (uint16_t)PCD_GET_EP_DBUF0_CNT(hpcd->Instance, ep->num);

    if (ep->xfer_len > TxByteNbre)
    {
      ep->xfer_len -= TxByteNbre;
    }
    else
    {
      ep->xfer_len = 0U;
    }
    /* Transfer is completed */
    if (ep->xfer_len == 0U)
    {
      /* TX COMPLETE */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
      hpcd->DataInStageCallback(hpcd, ep->num);
#else
      HAL_PCD_DataInStageCallback(hpcd, ep->num);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */

      if ((wEPVal & USB_EP_DTOG_RX) != 0U)
      {
        PCD_FreeUserBuffer(hpcd->Instance, ep->num, 1U);
      }
    }
    else /* Transfer is not yet Done */
    {
      /* need to Free USB Buff */
      if ((wEPVal & USB_EP_DTOG_RX) != 0U)
      {
        PCD_FreeUserBuffer(hpcd->Instance, ep->num, 1U);
      }

      /* Still there is data to Fill in the next Buffer */
      if (ep->xfer_fill_db == 1U)
      {
        ep->xfer_buff += TxByteNbre;
        ep->xfer_count += TxByteNbre;

        /* Calculate the len of the new buffer to fill */
        if (ep->xfer_len_db >= ep->maxpacket)
        {
          len = ep->maxpacket;
          ep->xfer_len_db -= len;
        }
        else if (ep->xfer_len_db == 0U)
        {
          len = TxByteNbre;
          ep->xfer_fill_db = 0U;
        }
        else
        {
          ep->xfer_fill_db = 0U;
          len = ep->xfer_len_db;
          ep->xfer_len_db = 0U;
        }

        /* Write remaining Data to Buffer */
        /* Set the Double buffer counter for pma buffer1 */
        PCD_SET_EP_DBUF0_CNT(hpcd->Instance, ep->num, ep->is_in, len);

        /* Copy user buffer to USB PMA */
        USB_WritePMA(hpcd->Instance, ep->xfer_buff,  ep->pmaaddr0, (uint16_t)len);
      }
    }
  }
  else /* Data Buffer1 ACK received */
  {
    /* multi-packet on the NON control IN endpoint */
    TxByteNbre = (uint16_t)PCD_GET_EP_DBUF1_CNT(hpcd->Instance, ep->num);

    if (ep->xfer_len >= TxByteNbre)
    {
      ep->xfer_len -= TxByteNbre;
    }
    else
    {
      ep->xfer_len = 0U;
    }

    /* Transfer is completed */
    if (ep->xfer_len == 0U)
    {
      /* TX COMPLETE */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
      hpcd->DataInStageCallback(hpcd, ep->num);
#else
      HAL_PCD_DataInStageCallback(hpcd, ep->num);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */

      /*need to Free USB Buff*/
      if ((wEPVal & USB_EP_DTOG_RX) == 0U)
      {
        PCD_FreeUserBuffer(hpcd->Instance, ep->num, 1U);
      }
    }
    else /* Transfer is not yet Done */
    {
      /* need to Free USB Buff */
      if ((wEPVal & USB_EP_DTOG_RX) == 0U)
      {
        PCD_FreeUserBuffer(hpcd->Instance, ep->num, 1U);
      }

      /* Still there is data to Fill in the next Buffer */
      if (ep->xfer_fill_db == 1U)
      {
        ep->xfer_buff += TxByteNbre;
        ep->xfer_count += TxByteNbre;

        /* Calculate the len of the new buffer to fill */
        if (ep->xfer_len_db >= ep->maxpacket)
        {
          len = ep->maxpacket;
          ep->xfer_len_db -= len;
        }
        else if (ep->xfer_len_db == 0U)
        {
          len = TxByteNbre;
          ep->xfer_fill_db = 0U;
        }
        else
        {
          len = ep->xfer_len_db;
          ep->xfer_len_db = 0U;
          ep->xfer_fill_db = 0;
        }

        /* Set the Double buffer counter for pmabuffer1 */
        PCD_SET_EP_DBUF1_CNT(hpcd->Instance, ep->num, ep->is_in, len);

        /* Copy the user buffer to USB PMA */
        USB_WritePMA(hpcd->Instance, ep->xfer_buff,  ep->pmaaddr1, (uint16_t)len);
      }
    }
  }

  /*enable endpoint IN*/
  PCD_SET_EP_TX_STATUS(hpcd->Instance, ep->num, USB_EP_TX_VALID);

  return HAL_OK;
}

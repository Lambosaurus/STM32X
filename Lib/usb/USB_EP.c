
#include "USB_EP.h"

#include "USB_PCD.h"

/*
 * PRIVATE DEFINITIONS
 */

#define USB_ENDPOINTS		8
#define BTABLE_SIZE			(USB_ENDPOINTS * 8)

#define PMA_SIZE			512
#define PMA_BASE			(((uint32_t)USB) + 0x400)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static uint16_t USB_PMA_Alloc(uint16_t size);
//TODO: Remove from header when able.
//static void USB_PMA_Write(uint16_t address, uint8_t * data, uint16_t count);
//static void USB_PMA_Read(uint16_t address, uint8_t * data, uint16_t count);

static PCD_EPTypeDef * USB_EP_GetEP(uint8_t endpoint);

//#ifdef USE_EP_DOUBLEBUFFER
//TODO: Fix doublebuffer partitioning
static uint16_t USB_EP_ReceiveDB(PCD_EPTypeDef *ep, uint16_t wEPVal);
static void USB_EP_TransmitDB(PCD_EPTypeDef *ep, uint16_t wEPVal);
//#endif



/*
 * PRIVATE VARIABLES
 */

static struct {
	uint16_t pma_head;
}gEP;

// DELETE THIS
PCD_HandleTypeDef * hpcd = &hpcd_USB_FS;

/*
 * PUBLIC FUNCTIONS
 */

void USB_EP_Init(void)
{
	gEP.pma_head = BTABLE_SIZE;

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
}

void USB_EP_Deinit(void)
{

}

void USB_EP_Open(uint8_t endpoint, uint8_t type, uint16_t size)
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
#ifdef USE_EP_DOUBLEBUFFER
	if (doublebuffer)
	{
		ep->doublebuffer = 1;
		ep->pmaaddr0 = USB_PMA_Alloc(size);
		ep->pmaaddr1 = USB_PMA_Alloc(size);
	}
	else
	{
		ep->pmaadress = USB_PMA_Alloc(size);
	}
#else
	ep->pmaadress = USB_PMA_Alloc(size);
#endif
	USB_ActivateEndpoint(USB, ep);
}

void USB_EP_Close(uint8_t endpoint)
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

void USB_EP_Rx(uint8_t endpoint, uint8_t *data, uint32_t count)
{
	PCD_EPTypeDef * ep = &(hpcd_USB_FS.OUT_ep[endpoint & EP_ADDR_MSK]);
	ep->xfer_buff = data;
	ep->xfer_len = count;
	ep->xfer_count = 0;
	USB_EPStartXfer(USB, ep);
}

void USB_EP_Tx(uint8_t endpoint, const uint8_t * data, uint32_t count)
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

void USB_EP_Stall(uint8_t endpoint)
{
	PCD_EPTypeDef *ep = USB_EP_GetEP(endpoint);
	ep->is_stall = 1U;
	if (ep->is_in)
	{
		PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_STALL);
	}
	else
	{
		PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_STALL);
	}
}

void USB_EP_Destall(uint8_t endpoint)
{
	PCD_EPTypeDef *ep = USB_EP_GetEP(endpoint);
	ep->is_stall = 0U;
	if (!ep->doublebuffer)
	{
		if (ep->is_in)
		{
			PCD_CLEAR_TX_DTOG(USB, ep->num);
			if (ep->type != EP_TYPE_ISOC)
			{
				PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_NAK);
			}
		}
		else
		{
			PCD_CLEAR_RX_DTOG(USB, ep->num);
			PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_VALID);
		}
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static PCD_EPTypeDef * USB_EP_GetEP(uint8_t endpoint)
{
	if (endpoint & 0x80U)
	{
		return &hpcd->IN_ep[endpoint & EP_ADDR_MSK];
	}
	else
	{
		return &hpcd->OUT_ep[endpoint & EP_ADDR_MSK];
	}
}

static uint16_t USB_PMA_Alloc(uint16_t size)
{
	uint16_t head = gEP.pma_head;
	gEP.pma_head += size;
	if (gEP.pma_head > PMA_SIZE)
	{
		// TODO: fix this?
		__BKPT();
	}
	return head;
}

void USB_PMA_Write(uint16_t address, uint8_t * data, uint16_t count)
{
	uint16_t * __restrict pma = (uint16_t * __restrict)(PMA_BASE + ((uint32_t)address * PMA_ACCESS));
	uint32_t words = (count + 1) / 2;
	while(words--)
	{
		uint32_t b1 = *data++;
		uint32_t b2 = *data++;
		*pma = b1 | (b2 << 8);
		pma += PMA_ACCESS;
	}
}

void USB_PMA_Read(uint16_t address, uint8_t * data, uint16_t count)
{
	uint16_t * __restrict pma = (uint16_t * __restrict)(PMA_BASE + ((uint32_t)address * PMA_ACCESS));
	uint32_t words = count / 2;

	while (words--)
	{
		uint32_t word = *pma;
		pma += PMA_ACCESS;
		*data++ = (uint8_t)word;
		*data++ = (uint8_t)(word >> 8);
	}

	if (count & 0x01)
	{
		uint32_t word = *pma;
		*data = (uint8_t)word;
	}
}

static uint16_t USB_EP_ReceiveDB(PCD_EPTypeDef *ep, uint16_t wEPVal)
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
      USB_PMA_Read(ep->pmaaddr0, ep->xfer_buff, count);
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
      USB_PMA_Read(ep->pmaaddr1, ep->xfer_buff, count);
    }
  }

  return count;
}

static void USB_EP_TransmitDB(PCD_EPTypeDef *ep, uint16_t wEPVal)
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
      HAL_PCD_DataInStageCallback(hpcd, ep->num);

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
        USB_PMA_Write(ep->pmaaddr0, ep->xfer_buff, (uint16_t)len);
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
      HAL_PCD_DataInStageCallback(hpcd, ep->num);

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
        USB_PMA_Write(ep->pmaaddr1, ep->xfer_buff, (uint16_t)len);
      }
    }
  }

  PCD_SET_EP_TX_STATUS(hpcd->Instance, ep->num, USB_EP_TX_VALID);
}

/*
 * INTERRUPT ROUTINES
 */

void USB_EP_IRQHandler(void)
{
  while (hpcd->Instance->ISTR & USB_ISTR_CTR)
  {
    uint32_t istr = hpcd->Instance->ISTR;
    uint8_t epnum = (uint8_t)(istr & USB_ISTR_EP_ID);

    if (epnum == 0)
    {
      // Control endpoint

      if ((istr & USB_ISTR_DIR) == 0)
      {
        // IN endpoint

        PCD_CLEAR_TX_EP_CTR(USB, epnum);
        PCD_EPTypeDef * ep = &hpcd->IN_ep[epnum];

        ep->xfer_count = PCD_GET_EP_TX_CNT(USB, ep->num);
        ep->xfer_buff += ep->xfer_count;

        HAL_PCD_DataInStageCallback(hpcd, ep->num);

        // DELETE THIS GARBAGE.
        if ((hpcd->USB_Address > 0U) && (ep->xfer_len == 0U))
        {
          hpcd->Instance->DADDR = ((uint16_t)hpcd->USB_Address | USB_DADDR_EF);
          hpcd->USB_Address = 0U;
        }
      }
      else
      {
        // OUT endpoint
    	PCD_EPTypeDef * ep = &hpcd->OUT_ep[epnum];
        uint16_t epReg = PCD_GET_ENDPOINT(hpcd->Instance, PCD_ENDP0);
        ep->xfer_count = PCD_GET_EP_RX_CNT(USB, ep->num);

        if (epReg & USB_EP_SETUP)
        {
          USB_PMA_Read(ep->pmaadress, (uint8_t *)hpcd->Setup, ep->xfer_count);

          // SETUP bit kept frozen while CTR_RX
          PCD_CLEAR_RX_EP_CTR(USB, 0);
          HAL_PCD_SetupStageCallback(hpcd);
        }
        else if (epReg & USB_EP_CTR_RX)
        {
          PCD_CLEAR_RX_EP_CTR(USB, 0);

          if ((ep->xfer_count != 0U) && (ep->xfer_buff != 0U))
          {
            USB_PMA_Read(ep->pmaadress, ep->xfer_buff, ep->xfer_count);

            ep->xfer_buff += ep->xfer_count;
            HAL_PCD_DataOutStageCallback(hpcd, 0U);
          }

          PCD_SET_EP_RX_CNT(USB, 0, ep->maxpacket);
          PCD_SET_EP_RX_STATUS(USB, 0, USB_EP_RX_VALID);
        }
      }
    }
    else
    {
    	// Other endpoints
      uint16_t epReg = PCD_GET_ENDPOINT(USB, epnum);

      if (epReg & USB_EP_CTR_RX)
      {
        PCD_CLEAR_RX_EP_CTR(USB, epnum);
        PCD_EPTypeDef * ep = &hpcd->OUT_ep[epnum];

        uint16_t count;

        if (ep->doublebuffer == 0U)
        {
          count = PCD_GET_EP_RX_CNT(USB, ep->num);
          if (count)
          {
            USB_PMA_Read(ep->pmaadress, ep->xfer_buff, count);
          }
        }
        else // Double buffered
        {
          if (ep->type == EP_TYPE_BULK)
          {
             count = USB_EP_ReceiveDB(ep, epReg);
          }
          else // Double buffered Iso out
          {
            /* free EP OUT Buffer */
            PCD_FreeUserBuffer(USB, ep->num, 0);

            if (PCD_GET_ENDPOINT(USB, ep->num) & USB_EP_DTOG_RX)
            {
              count = (uint16_t)PCD_GET_EP_DBUF0_CNT(USB, ep->num);

              if (count)
              {
                 USB_PMA_Read(ep->pmaaddr0, ep->xfer_buff, count);
              }
            }
            else
            {
              /* read from endpoint BUF1Addr buffer */
              count = (uint16_t)PCD_GET_EP_DBUF1_CNT(hpcd->Instance, ep->num);

              if (count)
              {
                 USB_PMA_Read(ep->pmaaddr1, ep->xfer_buff, count);
              }
            }
          }
        }
        ep->xfer_count += count;
        ep->xfer_buff += count;

        if ((ep->xfer_len == 0U) || (count < ep->maxpacket))
        {
          HAL_PCD_DataOutStageCallback(hpcd, ep->num);
        }
        else
        {
          USB_EPStartXfer(USB, ep);
        }
      }

      if (epReg & USB_EP_CTR_TX)
      {
    	PCD_EPTypeDef * ep = &hpcd->IN_ep[epnum];
        PCD_CLEAR_TX_EP_CTR(USB, epnum);

        /* Manage all non bulk transaction or Bulk Single Buffer Transaction */
        if (   (ep->type != EP_TYPE_BULK)
            || ((ep->type == EP_TYPE_BULK) && ((epReg & USB_EP_KIND) == 0U)))
        {
          uint16_t count = (uint16_t)PCD_GET_EP_TX_CNT(USB, ep->num);

          ep->xfer_len = ep->xfer_len > count ? ep->xfer_len - count : 0;

          if (ep->xfer_len == 0U)
          {
            // ZLP indicates TX complete
            HAL_PCD_DataInStageCallback(hpcd, ep->num);
          }
          else
          {
            /* Transfer is not yet Done */
            ep->xfer_buff += count;
            ep->xfer_count += count;
            USB_EPStartXfer(USB, ep);
          }
        }
        else
        {
          USB_EP_TransmitDB(ep, epReg);
        }
      }
    }
  }
}

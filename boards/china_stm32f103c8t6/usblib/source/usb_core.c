/**
  ******************************************************************************
  * @file    usb_core.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    28-August-2012
  * @brief   Standard protocol processing (USB v2.0)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ValBit(VAR,Place)    (VAR & (1 << Place))
#define SetBit(VAR,Place)    (VAR |= (1 << Place))
#define ClrBit(VAR,Place)    (VAR &= ((1 << Place) ^ 255))
#define Send0LengthData() { _SetEPTxCount(ENDP0, 0); \
    vSetEPTxStatus(EP_TX_VALID); \
  }

#define vSetEPRxStatus(st) (ep0_rx_state = st)
#define vSetEPTxStatus(st) (ep0_tx_state = st)

#define USB_StatusIn() Send0LengthData()
#define USB_StatusOut() vSetEPRxStatus(EP_RX_VALID)

#define StatusInfo0 StatusInfo.bw.bb1 /* Reverse bb0 & bb1 */
#define StatusInfo1 StatusInfo.bw.bb0

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t_uint8_t StatusInfo;

bool Data_Mul_MaxPacketSize = FALSE;
/* Private function prototypes -----------------------------------------------*/
static void DataStageOut(void);
static void DataStageIn(void);
static void setup_without_data_process(void);
static void setup_with_data_process(void);
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Standard_GetConfiguration.
* Description    : Return the current configuration variable address.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 1 , if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
uint8_t *Standard_GetConfiguration(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength =
      sizeof(pInformation->Current_Configuration);
    return 0;
  }
  pUser_Standard_Requests->User_GetConfiguration();
  return (uint8_t *)&pInformation->Current_Configuration;
}

/*******************************************************************************
* Function Name  : Standard_SetConfiguration.
* Description    : This routine is called to set the configuration value
*                  Then each class should configure device itself.
* Input          : None.
* Output         : None.
* Return         : Return USB_SUCCESS, if the request is performed.
*                  Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetConfiguration(void)
{

  if ((pInformation->USBwValue0 <=
      Device_Table.Total_Configuration) && (pInformation->USBwValue1 == 0)
      && (pInformation->USBwIndex == 0)) /*call Back usb spec 2.0*/
  {
    pInformation->Current_Configuration = pInformation->USBwValue0;
    pUser_Standard_Requests->User_SetConfiguration();
    return USB_SUCCESS;
  }
  else
  {
    return USB_UNSUPPORT;
  }
}

/*******************************************************************************
* Function Name  : Standard_GetInterface.
* Description    : Return the Alternate Setting of the current interface.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
uint8_t *Standard_GetInterface(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength =
      sizeof(pInformation->Current_AlternateSetting);
    return 0;
  }
  pUser_Standard_Requests->User_GetInterface();
  return (uint8_t *)&pInformation->Current_AlternateSetting;
}

/*******************************************************************************
* Function Name  : Standard_SetInterface.
* Description    : This routine is called to set the interface.
*                  Then each class should configure the interface them self.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetInterface(void)
{
  RESULT Re;
  /*Test if the specified Interface and Alternate Setting are supported by
    the application Firmware*/
  Re = (*pProperty->Class_Get_Interface_Setting)(pInformation->USBwIndex0, pInformation->USBwValue0);

  if (pInformation->Current_Configuration != 0)
  {
    if ((Re != USB_SUCCESS) || (pInformation->USBwIndex1 != 0)
        || (pInformation->USBwValue1 != 0))
    {
      return  USB_UNSUPPORT;
    }
    else if (Re == USB_SUCCESS)
    {
      pUser_Standard_Requests->User_SetInterface();
      pInformation->Current_Interface = pInformation->USBwIndex0;
      pInformation->Current_AlternateSetting = pInformation->USBwValue0;
      return USB_SUCCESS;
    }

  }

  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : Standard_GetStatus.
* Description    : Copy the device request data to "StatusInfo buffer".
* Input          : - Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is at end of data block,
*                  or is invalid when "Length" is 0.
*******************************************************************************/
uint8_t *Standard_GetStatus(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 2;
    return 0;
  }

  /* Reset Status Information */
  StatusInfo.w = 0;

  if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
  {
    /*Get Device Status */
    uint8_t Feature = pInformation->Current_Feature;

    /* Remote Wakeup enabled */
    if (ValBit(Feature, 5))
    {
      SetBit(StatusInfo0, 1);
    }
    else
    {
      ClrBit(StatusInfo0, 1);
    }      

    /* Bus-powered */
    if (ValBit(Feature, 6))
    {
      SetBit(StatusInfo0, 0);
    }
    else /* Self-powered */
    {
      ClrBit(StatusInfo0, 0);
    }
  }
  /*Interface Status*/
  else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
  {
    return (uint8_t *)&StatusInfo;
  }
  /*Get EndPoint Status*/
  else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
  {
    uint8_t Related_Endpoint;
    uint8_t wIndex0 = pInformation->USBwIndex0;

    Related_Endpoint = (wIndex0 & 0x0f);
    if (ValBit(wIndex0, 7))
    {
      /* IN endpoint */
      if (_GetTxStallStatus(Related_Endpoint))
      {
        SetBit(StatusInfo0, 0); /* IN Endpoint stalled */
      }
    }
    else
    {
      /* OUT endpoint */
      if (_GetRxStallStatus(Related_Endpoint))
      {
        SetBit(StatusInfo0, 0); /* OUT Endpoint stalled */
      }
    }

  }
  else
  {
    return NULL;
  }
  pUser_Standard_Requests->User_GetStatus();
  return (uint8_t *)&StatusInfo;
}

/*******************************************************************************
* Function Name  : Standard_ClearFeature.
* Description    : Clear or disable a specific feature.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_ClearFeature(void)
{
  uint32_t     Type_Rec = Type_Recipient;
  uint32_t     Status;


  if (Type_Rec == (STANDARD_REQUEST | DEVICE_RECIPIENT))
  {/*Device Clear Feature*/
    ClrBit(pInformation->Current_Feature, 5);
    return USB_SUCCESS;
  }
  else if (Type_Rec == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
  {/*EndPoint Clear Feature*/
    DEVICE* pDev;
    uint32_t Related_Endpoint;
    uint32_t wIndex0;
    uint32_t rEP;

    if ((pInformation->USBwValue != ENDPOINT_STALL)
        || (pInformation->USBwIndex1 != 0))
    {
      return USB_UNSUPPORT;
    }

    pDev = &Device_Table;
    wIndex0 = pInformation->USBwIndex0;
    rEP = wIndex0 & ~0x80;
    Related_Endpoint = ENDP0 + rEP;

    if (ValBit(pInformation->USBwIndex0, 7))
    {
      /*Get Status of endpoint & stall the request if the related_ENdpoint
      is Disabled*/
      Status = _GetEPTxStatus(Related_Endpoint);
    }
    else
    {
      Status = _GetEPRxStatus(Related_Endpoint);
    }

    if ((rEP >= pDev->Total_Endpoint) || (Status == 0)
        || (pInformation->Current_Configuration == 0))
    {
      return USB_UNSUPPORT;
    }


    if (wIndex0 & 0x80)
    {
      /* IN endpoint */
      if (_GetTxStallStatus(Related_Endpoint ))
      {
        ClearDTOG_TX(Related_Endpoint);
        SetEPTxStatus(Related_Endpoint, EP_TX_VALID);
      }
    }
    else
    {
      /* OUT endpoint */
      if (_GetRxStallStatus(Related_Endpoint))
      {
        if (Related_Endpoint == ENDP0)
        {
          /* After clear the STALL, enable the default endpoint receiver */
          SetEPRxCount(Related_Endpoint, Device_Property.MaxPacketSize);
          _SetEPRxStatus(Related_Endpoint, EP_RX_VALID);
        }
        else
        {
          ClearDTOG_RX(Related_Endpoint);
          _SetEPRxStatus(Related_Endpoint, EP_RX_VALID);
        }
      }
    }
    pUser_Standard_Requests->User_ClearFeature();
    return USB_SUCCESS;
  }

  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : Standard_SetEndPointFeature
* Description    : Set or enable a specific feature of EndPoint
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetEndPointFeature(void)
{
  uint32_t    wIndex0;
  uint32_t    Related_Endpoint;
  uint32_t    rEP;
  uint32_t    Status;

  wIndex0 = pInformation->USBwIndex0;
  rEP = wIndex0 & ~0x80;
  Related_Endpoint = ENDP0 + rEP;

  if (ValBit(pInformation->USBwIndex0, 7))
  {
    /* get Status of endpoint & stall the request if the related_ENdpoint
    is Disabled*/
    Status = _GetEPTxStatus(Related_Endpoint);
  }
  else
  {
    Status = _GetEPRxStatus(Related_Endpoint);
  }

  if (Related_Endpoint >= Device_Table.Total_Endpoint
      || pInformation->USBwValue != 0 || Status == 0
      || pInformation->Current_Configuration == 0)
  {
    return USB_UNSUPPORT;
  }
  else
  {
    if (wIndex0 & 0x80)
    {
      /* IN endpoint */
      _SetEPTxStatus(Related_Endpoint, EP_TX_STALL);
    }

    else
    {
      /* OUT endpoint */
      _SetEPRxStatus(Related_Endpoint, EP_RX_STALL);
    }
  }
  pUser_Standard_Requests->User_SetEndPointFeature();
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Standard_SetDeviceFeature.
* Description    : Set or enable a specific feature of Device.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetDeviceFeature(void)
{
  SetBit(pInformation->Current_Feature, 5);
  pUser_Standard_Requests->User_SetDeviceFeature();
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Standard_GetDescriptorData.
* Description    : Standard_GetDescriptorData is used for descriptors transfer.
*                : This routine is used for the descriptors resident in Flash
*                  or RAM
*                  pDesc can be in either Flash or RAM
*                  The purpose of this routine is to have a versatile way to
*                  response descriptors request. It allows user to generate
*                  certain descriptors with software or read descriptors from
*                  external storage part by part.
* Input          : - Length - Length of the data in this transfer.
*                  - pDesc - A pointer points to descriptor struct.
*                  The structure gives the initial address of the descriptor and
*                  its original size.
* Output         : None.
* Return         : Address of a part of the descriptor pointed by the Usb_
*                  wOffset The buffer pointed by this address contains at least
*                  Length bytes.
*******************************************************************************/
uint8_t *Standard_GetDescriptorData(uint16_t Length, ONE_DESCRIPTOR *pDesc)
{
  uint32_t  wOffset;

  wOffset = pInformation->Ctrl_Info.Usb_wOffset;
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = pDesc->Descriptor_Size - wOffset;
    return 0;
  }

  return pDesc->Descriptor + wOffset;
}

/*******************************************************************************
* Function Name  : DataStageOut.
* Description    : Data stage of a Control Write Transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void DataStageOut(void)
{
  ENDPOINT_INFO *pEPinfo = &pInformation->Ctrl_Info;
  uint32_t save_rLength;

  save_rLength = pEPinfo->Usb_rLength;

  if (pEPinfo->CopyData && save_rLength)
  {
    uint8_t *Buffer;
    uint32_t Length;

    Length = pEPinfo->PacketSize;
    if (Length > save_rLength)
    {
      Length = save_rLength;
    }

    Buffer = (*pEPinfo->CopyData)(Length);
    pEPinfo->Usb_rLength -= Length;
    pEPinfo->Usb_rOffset += Length;
    PMAToUserBufferCopy(Buffer, GetEPRxAddr(ENDP0), Length);

  }

  if (pEPinfo->Usb_rLength != 0)
  {
    vSetEPRxStatus(EP_RX_VALID);/* re-enable for next data reception */
    SetEPTxCount(ENDP0, 0);
    vSetEPTxStatus(EP_TX_VALID);/* Expect the host to abort the data OUT stage */
  }
  /* Set the next State*/
  if (pEPinfo->Usb_rLength >= pEPinfo->PacketSize)
  {
    pInformation->ControlState = OUT_DATA;
  }
  else
  {
    if (pEPinfo->Usb_rLength > 0)
    {
      pInformation->ControlState = LAST_OUT_DATA;
    }
    else if (pEPinfo->Usb_rLength == 0)
    {
      pInformation->ControlState = WAIT_STATUS_IN;
      USB_StatusIn();
    }
  }
}

/*******************************************************************************
* Function Name  : DataStageIn.
* Description    : Data stage of a Control Read Transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void DataStageIn(void)
{
  ENDPOINT_INFO *pEPinfo = &pInformation->Ctrl_Info;
  uint32_t save_wLength = pEPinfo->Usb_wLength;
  uint32_t ControlState = pInformation->ControlState;

  uint8_t *DataBuffer;
  uint32_t Length;

  if ((save_wLength == 0) && (ControlState == LAST_IN_DATA))
  {
    if(Data_Mul_MaxPacketSize == TRUE)
    {
      /* No more data to send and empty packet */
      Send0LengthData();
      ControlState = LAST_IN_DATA;
      Data_Mul_MaxPacketSize = FALSE;
    }
    else 
    {
      /* No more data to send so STALL the TX Status*/
      ControlState = WAIT_STATUS_OUT;
      vSetEPTxStatus(EP_TX_STALL);
 
    }
    
    goto Expect_Status_Out;
  }

  Length = pEPinfo->PacketSize;
  ControlState = (save_wLength <= Length) ? LAST_IN_DATA : IN_DATA;

  if (Length > save_wLength)
  {
    Length = save_wLength;
  }

  DataBuffer = (*pEPinfo->CopyData)(Length);
  
  UserToPMABufferCopy(DataBuffer, GetEPTxAddr(ENDP0), Length);

  SetEPTxCount(ENDP0, Length);

  pEPinfo->Usb_wLength -= Length;
  pEPinfo->Usb_wOffset += Length;
  vSetEPTxStatus(EP_TX_VALID);

  USB_StatusOut();/* Expect the host to abort the data IN stage */

Expect_Status_Out:
  pInformation->ControlState = ControlState;
}

/*******************************************************************************
* Function Name  : NoData_Setup0.
* Description    : Proceed the processing of setup request without data stage.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void setup_without_data_process(void)
{
	RESULT		result = USB_UNSUPPORT;
	uint32_t	control_state;
	uint32_t	request_number = pInformation->b_request;
	uint8_t		request_type = pInformation->bm_request_type & (REQUEST_TYPE | RECIPIENT);

	/************************************************/
	/************************************************/

	//if (request_type == (STANDARD_REQUEST | DEVICE_RECIPIENT)) {
	if (request_type == DEVICE_RECIPIENT) {
	/* Device Request*/

		/************************************************/
		/* SET_CONFIGURATION*/
		if (request_number == SET_CONFIGURATION) {
			result = Standard_SetConfiguration();
		}

		/************************************************/
		/*SET ADDRESS*/
		else if (request_number == SET_ADDRESS) {

			if ((pInformation->USBwValue0 > 127) ||
				(pInformation->USBwValue1 != 0) ||
				(pInformation->USBwIndex != 0) ||
				(pInformation->Current_Configuration != 0)) {

				/* Device Address should be 127 or less*/
				control_state = STALLED;
				pInformation->ControlState = control_state;
				return;

			} else {
				result = USB_SUCCESS;
			}
		}

		/************************************************/
		/*SET FEATURE for Device*/
		else if (request_number == SET_FEATURE) {

			if ((pInformation->USBwValue0 == DEVICE_REMOTE_WAKEUP) &&
				(pInformation->USBwIndex == 0)) {

				result = Standard_SetDeviceFeature();

			} else {
				result = USB_UNSUPPORT;
			}
		}

		/************************************************/
		/*Clear FEATURE for Device */
		else if (request_number == CLEAR_FEATURE) {

			if (pInformation->USBwValue0 == DEVICE_REMOTE_WAKEUP &&
				pInformation->USBwIndex == 0 &&
				(pInformation->Current_Feature & (1 << 5))) {

				result = Standard_ClearFeature();

			} else {
				result = USB_UNSUPPORT;
			}
		}
	}

	/************************************************/
	/************************************************/
	/* Interface Request*/
	else if (request_type == INTERFACE_RECIPIENT) {

		/************************************************/
		/*SET INTERFACE*/
		if (request_number == SET_INTERFACE) {
			result = Standard_SetInterface();
		}
	}

	/************************************************/
	/************************************************/
	/* EndPoint Request*/
	else if (request_type == ENDPOINT_RECIPIENT) {

		/************************************************/
		/*CLEAR FEATURE for EndPoint*/
		if (request_number == CLEAR_FEATURE) {
			result = Standard_ClearFeature();
		}
		/************************************************/
		/* SET FEATURE for EndPoint*/
		else if (request_number == SET_FEATURE) {
			result = Standard_SetEndPointFeature();
		}
	}

	/************************************************/
	/************************************************/
	else {
		result = USB_UNSUPPORT;
	}

	if (result != USB_SUCCESS) {
		result = (*pProperty->Class_NoData_Setup)(request_number);

		if (result == USB_NOT_READY) {
			control_state = PAUSE;
			pInformation->ControlState = control_state;
			return;
		}
	}

	if (result != USB_SUCCESS) {
		control_state = STALLED;
		pInformation->ControlState = control_state;
		return;
	}

	/* After no data stage SETUP */
	control_state = WAIT_STATUS_IN;
	USB_StatusIn();

	pInformation->ControlState = control_state;
}

/*******************************************************************************
* Function Name  : Data_Setup0.
* Description    : Proceed the processing of setup request with data stage.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void setup_with_data_process(void)
{
  uint8_t *(*CopyRoutine)(uint16_t);
  RESULT Result;
  uint32_t Request_No = pInformation->b_request;

  uint32_t Related_Endpoint, Reserved;
  uint32_t wOffset, Status;



  CopyRoutine = NULL;
  wOffset = 0;

  /*GET DESCRIPTOR*/
  if (Request_No == GET_DESCRIPTOR)
  {
    if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
    {
      uint8_t wValue1 = pInformation->USBwValue1;
      if (wValue1 == DEVICE_DESCRIPTOR)
      {
        CopyRoutine = pProperty->GetDeviceDescriptor;
      }
      else if (wValue1 == CONFIG_DESCRIPTOR)
      {
        CopyRoutine = pProperty->GetConfigDescriptor;
      }
      else if (wValue1 == STRING_DESCRIPTOR)
      {
        CopyRoutine = pProperty->GetStringDescriptor;
      }  /* End of GET_DESCRIPTOR */
    }
  }

  /*GET STATUS*/
  else if ((Request_No == GET_STATUS) && (pInformation->USBwValue == 0)
           && (pInformation->USBwLength == 0x0002)
           && (pInformation->USBwIndex1 == 0))
  {
    /* GET STATUS for Device*/
    if ((Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
        && (pInformation->USBwIndex == 0))
    {
      CopyRoutine = Standard_GetStatus;
    }

    /* GET STATUS for Interface*/
    else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
    {
      if (((*pProperty->Class_Get_Interface_Setting)(pInformation->USBwIndex0, 0) == USB_SUCCESS)
          && (pInformation->Current_Configuration != 0))
      {
        CopyRoutine = Standard_GetStatus;
      }
    }

    /* GET STATUS for EndPoint*/
    else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
    {
      Related_Endpoint = (pInformation->USBwIndex0 & 0x0f);
      Reserved = pInformation->USBwIndex0 & 0x70;

      if (ValBit(pInformation->USBwIndex0, 7))
      {
        /*Get Status of endpoint & stall the request if the related_ENdpoint
        is Disabled*/
        Status = _GetEPTxStatus(Related_Endpoint);
      }
      else
      {
        Status = _GetEPRxStatus(Related_Endpoint);
      }

      if ((Related_Endpoint < Device_Table.Total_Endpoint) && (Reserved == 0)
          && (Status != 0))
      {
        CopyRoutine = Standard_GetStatus;
      }
    }

  }

  /*GET CONFIGURATION*/
  else if (Request_No == GET_CONFIGURATION)
  {
    if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
    {
      CopyRoutine = Standard_GetConfiguration;
    }
  }
  /*GET INTERFACE*/
  else if (Request_No == GET_INTERFACE)
  {
    if ((Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
        && (pInformation->Current_Configuration != 0) && (pInformation->USBwValue == 0)
        && (pInformation->USBwIndex1 == 0) && (pInformation->USBwLength == 0x0001)
        && ((*pProperty->Class_Get_Interface_Setting)(pInformation->USBwIndex0, 0) == USB_SUCCESS))
    {
      CopyRoutine = Standard_GetInterface;
    }

  }
  
  if (CopyRoutine)
  {
    pInformation->Ctrl_Info.Usb_wOffset = wOffset;
    pInformation->Ctrl_Info.CopyData = CopyRoutine;
    /* sb in the original the cast to word was directly */
    /* now the cast is made step by step */
    (*CopyRoutine)(0);
    Result = USB_SUCCESS;
  }
  else
  {
    Result = (*pProperty->Class_Data_Setup)(pInformation->b_request);
    if (Result == USB_NOT_READY)
    {
      pInformation->ControlState = PAUSE;
      return;
    }
  }

  if (pInformation->Ctrl_Info.Usb_wLength == 0xFFFF)
  {
    /* Data is not ready, wait it */
    pInformation->ControlState = PAUSE;
    return;
  }
  if ((Result == USB_UNSUPPORT) || (pInformation->Ctrl_Info.Usb_wLength == 0))
  {
    /* Unsupported request */
    pInformation->ControlState = STALLED;
    return;
  }


  if (ValBit(pInformation->bm_request_type, 7))
  {
    /* Device ==> Host */
    __IO uint32_t wLength = pInformation->USBwLength;
     
    /* Restrict the data length to be the one host asks for */
    if (pInformation->Ctrl_Info.Usb_wLength > wLength)
    {
      pInformation->Ctrl_Info.Usb_wLength = wLength;
    }
    
    else if (pInformation->Ctrl_Info.Usb_wLength < pInformation->USBwLength)
    {
      if (pInformation->Ctrl_Info.Usb_wLength < pProperty->MaxPacketSize)
      {
        Data_Mul_MaxPacketSize = FALSE;
      }
      else if ((pInformation->Ctrl_Info.Usb_wLength % pProperty->MaxPacketSize) == 0)
      {
        Data_Mul_MaxPacketSize = TRUE;
      }
    }   

    pInformation->Ctrl_Info.PacketSize = pProperty->MaxPacketSize;
    DataStageIn();
  }
  else
  {
    pInformation->ControlState = OUT_DATA;
    vSetEPRxStatus(EP_RX_VALID); /* enable for next data reception */
  }

  return;
}

/*******************************************************************************
* Function Name  : Setup0_Process
* Description    : Get the device request data and dispatch to individual process.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
uint8_t ep0_setup_process(void)
{
	struct std_request {
		uint8_t		bmRequestType;
		uint8_t		bRequest;
		uint16_t	wValue;
		uint16_t	wIndex;
		uint16_t	wLength;
	};

	struct std_request *prequest = (struct std_request *)(PMAAddr + (uint8_t *)(_GetEPRxAddr(ENDP0) * 2));

	if (pInformation->ControlState != PAUSE) {

		pInformation->bm_request_type = prequest->bmRequestType;

		pInformation->b_request = prequest->bRequest;

		pInformation->USBwValue = ByteSwap(prequest->wValue);

		pInformation->USBwIndex	= ByteSwap(prequest->wIndex);

		pInformation->USBwLength = ByteSwap(prequest->wLength); /*TODO: USB check correctness of the swapping*/
	}

	pInformation->ControlState = SETTING_UP;

	if (pInformation->USBwLength == 0) {
		setup_without_data_process();

	} else {
		setup_with_data_process();
	}

	return ep0_finish_processing();


//	union {
//		uint8_t		*b;
//		uint16_t	*w;
//	} pBuf;
//
//	uint16_t offset = 1;
//
//	pBuf.b = PMAAddr + (uint8_t *)(_GetEPRxAddr(ENDP0) * 2); /* *2 for 32 bits addr */
//
//	if (pInformation->ControlState != PAUSE) {
//		pInformation->bm_request_type = *pBuf.b++; /* bmRequestType */
//		pInformation->b_request = *pBuf.b++; /* bRequest */
//		pBuf.w += offset;  /* word not accessed because of 32 bits addressing */
//		pInformation->USBwValue = ByteSwap(*pBuf.w++); /* wValue */
//		pBuf.w += offset;  /* word not accessed because of 32 bits addressing */
//		pInformation->USBwIndex  = ByteSwap(*pBuf.w++); /* wIndex */
//		pBuf.w += offset;  /* word not accessed because of 32 bits addressing */
//		pInformation->USBwLength = *pBuf.w; /* wLength */
//	}
//
//	pInformation->ControlState = SETTING_UP;
//
//	if (pInformation->USBwLength == 0) {
//
//		NoData_Setup0();
//	} else {
//
//		Data_Setup0();
//	}
//
//	return ep0_finish_processing();
}

/*******************************************************************************
* Function Name  : In0_Process
* Description    : Process the IN token on all default endpoint.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
uint8_t ep0_in_process(void)
{
	uint32_t ControlState = pInformation->ControlState;

	if ((ControlState == IN_DATA) || (ControlState == LAST_IN_DATA)) {

		DataStageIn();
		/* ControlState may be changed outside the function */
		ControlState = pInformation->ControlState;

	} else if (ControlState == WAIT_STATUS_IN) {

		if ((pInformation->b_request == SET_ADDRESS) &&
			(Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))) {

			SetDeviceAddress(pInformation->USBwValue0);
			pUser_Standard_Requests->User_SetDeviceAddress();
		}

		(*pProperty->Process_Status_IN)();
		ControlState = STALLED;

	} else {
		ControlState = STALLED;
	}

	pInformation->ControlState = ControlState;

	return ep0_finish_processing();
}

/*******************************************************************************
* Function Name  : Out0_Process
* Description    : Process the OUT token on all default endpoint.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
uint8_t ep0_out_process(void)
{
	uint32_t ControlState = pInformation->ControlState;

	if ((ControlState == IN_DATA) || (ControlState == LAST_IN_DATA)) {
		/* host aborts the transfer before finish */
		ControlState = STALLED;

	} else if ((ControlState == OUT_DATA) || (ControlState == LAST_OUT_DATA)) {

		DataStageOut();
		ControlState = pInformation->ControlState; /* may be changed outside the function */

	} else if (ControlState == WAIT_STATUS_OUT) {

		(*pProperty->Process_Status_OUT)();
		ControlState = STALLED;

	} else {
		ControlState = STALLED;
	}

	pInformation->ControlState = ControlState;

	return ep0_finish_processing();
}

/*******************************************************************************
* Function Name  : Post0_Process
* Description    : Stall the Endpoint 0 in case of error.
* Input          : None.
* Output         : None.
* Return         : - 0 if the control State is in PAUSE
*                  - 1 if not.
*******************************************************************************/
uint8_t ep0_finish_processing(void)
{
	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);

	if (pInformation->ControlState == STALLED) {
		vSetEPRxStatus(EP_RX_STALL);
		vSetEPTxStatus(EP_TX_STALL);
	}

	return (pInformation->ControlState == PAUSE);
}

/*******************************************************************************
* Function Name  : SetDeviceAddress.
* Description    : Set the device and all the used Endpoints addresses.
* Input          : - Val: device address.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetDeviceAddress(uint8_t Val)
{
	uint32_t i;
	uint32_t nEP = Device_Table.Total_Endpoint;

	/* set address in every used endpoint */
	for (i = 0; i < nEP; i++) {
		_SetEPAddress((uint8_t)i, (uint8_t)i);
	}

	_SetDADDR(Val | DADDR_EF); /* set device address and enable function */
}

/*******************************************************************************
* Function Name  : NOP_Process
* Description    : No operation function.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void NOP_Process(void)
{
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

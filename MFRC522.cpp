#include "MFRC522.h"

MFRC522::MFRC522(PinName mosi, PinName miso, PinName sclk, PinName cs,
                 PinName reset)
    : _spi(mosi, miso, sclk), _cs(cs), _reset(reset) {
  _spi.format(8, 0);
  _spi.frequency(1000000);
  _cs = 1;
  _reset = 1;
}

void MFRC522::PCD_Init() {
  _reset = 0;
  wait_us(50000);
  _reset = 1;
  wait_us(50000);

  PCD_Reset();

  PCD_WriteRegister(TModeReg, 0x8D);
  PCD_WriteRegister(TPrescalerReg, 0x3E);
  PCD_WriteRegister(TReloadRegL, 30);
  PCD_WriteRegister(TReloadRegH, 0);
  PCD_WriteRegister(TxASKReg, 0x40);
  PCD_WriteRegister(ModeReg, 0x3D);

  PCD_AntennaOn();
}

void MFRC522::PCD_Reset() {
  PCD_WriteRegister(CommandReg, PCD_SoftReset);

  uint8_t count = 0;
  do {
    wait_us(50000);
  } while ((PCD_ReadRegister(CommandReg) & (1 << 4)) && (++count) < 3);
}

void MFRC522::PCD_AntennaOn() {
  uint8_t value = PCD_ReadRegister(TxControlReg);
  if ((value & 0x03) != 0x03) {
    PCD_WriteRegister(TxControlReg, value | 0x03);
  }
}

void MFRC522::PCD_AntennaOff() { PCD_ClearRegisterBitMask(TxControlReg, 0x03); }

uint8_t MFRC522::PCD_GetAntennaGain() {
  return PCD_ReadRegister(RFCfgReg) & (0x07 << 4);
}

void MFRC522::PCD_SetAntennaGain(uint8_t mask) {
  if (PCD_GetAntennaGain() != mask) {
    PCD_ClearRegisterBitMask(RFCfgReg, (0x07 << 4));
    PCD_SetRegisterBitMask(RFCfgReg, mask & (0x07 << 4));
  }
}

void MFRC522::PCD_WriteRegister(uint8_t reg, uint8_t value) {
  _cs = 0;
  _spi.write((reg << 1) & 0x7E);
  _spi.write(value);
  _cs = 1;
}

void MFRC522::PCD_WriteRegister(uint8_t reg, uint8_t count, uint8_t *values) {
  _cs = 0;
  _spi.write((reg << 1) & 0x7E);
  for (uint8_t i = 0; i < count; i++) {
    _spi.write(values[i]);
  }
  _cs = 1;
}

uint8_t MFRC522::PCD_ReadRegister(uint8_t reg) {
  uint8_t value;
  _cs = 0;
  _spi.write(((reg << 1) & 0x7E) | 0x80);
  value = _spi.write(0);
  _cs = 1;
  return value;
}

void MFRC522::PCD_ReadRegister(uint8_t reg, uint8_t count, uint8_t *values,
                               uint8_t rxAlign) {
  if (count == 0)
    return;

  uint8_t address = 0x80 | ((reg << 1) & 0x7E);
  uint8_t index = 0;

  _cs = 0;
  count--;
  _spi.write(address);

  if (rxAlign) {
    uint8_t mask = (0xFF << rxAlign) & 0xFF;
    uint8_t value = _spi.write(address);
    values[0] = (values[0] & ~mask) | (value & mask);
    index++;
  }

  while (index < count) {
    values[index] = _spi.write(address);
    index++;
  }

  values[index] = _spi.write(0);
  _cs = 1;
}

void MFRC522::PCD_SetRegisterBitMask(uint8_t reg, uint8_t mask) {
  uint8_t tmp = PCD_ReadRegister(reg);
  PCD_WriteRegister(reg, tmp | mask);
}

void MFRC522::PCD_ClearRegisterBitMask(uint8_t reg, uint8_t mask) {
  uint8_t tmp = PCD_ReadRegister(reg);
  PCD_WriteRegister(reg, tmp & (~mask));
}

MFRC522::StatusCode MFRC522::PCD_CalculateCRC(uint8_t *data, uint8_t length,
                                              uint8_t *result) {
  PCD_WriteRegister(CommandReg, PCD_Idle);
  PCD_WriteRegister(DivIrqReg, 0x04);
  PCD_WriteRegister(FIFOLevelReg, 0x80);
  PCD_WriteRegister(FIFODataReg, length, data);
  PCD_WriteRegister(CommandReg, PCD_CalcCRC);

  for (uint16_t i = 5000; i > 0; i--) {
    uint8_t n = PCD_ReadRegister(DivIrqReg);
    if (n & 0x04) {
      PCD_WriteRegister(CommandReg, PCD_Idle);
      result[0] = PCD_ReadRegister(CRCResultRegL);
      result[1] = PCD_ReadRegister(CRCResultRegH);
      return STATUS_OK;
    }
  }
  return STATUS_TIMEOUT;
}

MFRC522::StatusCode MFRC522::PCD_TransceiveData(
    uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen,
    uint8_t *validBits, uint8_t rxAlign, bool checkCRC) {
  uint8_t waitIRq = 0x30;
  return PCD_CommunicateWithPICC(PCD_Transceive, waitIRq, sendData, sendLen,
                                 backData, backLen, validBits, rxAlign,
                                 checkCRC);
}

MFRC522::StatusCode MFRC522::PCD_CommunicateWithPICC(
    uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen,
    uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign,
    bool checkCRC) {
  uint8_t txLastBits = validBits ? *validBits : 0;
  uint8_t bitFraming = (rxAlign << 4) + txLastBits;

  PCD_WriteRegister(CommandReg, PCD_Idle);
  PCD_WriteRegister(ComIrqReg, 0x7F);
  PCD_WriteRegister(FIFOLevelReg, 0x80);
  PCD_WriteRegister(FIFODataReg, sendLen, sendData);
  PCD_WriteRegister(BitFramingReg, bitFraming);
  PCD_WriteRegister(CommandReg, command);

  if (command == PCD_Transceive) {
    PCD_SetRegisterBitMask(BitFramingReg, 0x80);
  }

  uint16_t i;
  for (i = 2000; i > 0; i--) {
    uint8_t n = PCD_ReadRegister(ComIrqReg);
    if (n & waitIRq)
      break;
    if (n & 0x01)
      return STATUS_TIMEOUT;
  }

  if (i == 0)
    return STATUS_TIMEOUT;

  uint8_t errorRegValue = PCD_ReadRegister(ErrorReg);
  if (errorRegValue & 0x13)
    return STATUS_ERROR;

  uint8_t _validBits = 0;

  if (backData && backLen) {
    uint8_t n = PCD_ReadRegister(FIFOLevelReg);
    if (n > *backLen)
      return STATUS_NO_ROOM;

    *backLen = n;
    PCD_ReadRegister(FIFODataReg, n, backData, rxAlign);
    _validBits = PCD_ReadRegister(ControlReg) & 0x07;
    if (validBits)
      *validBits = _validBits;
  }

  if (errorRegValue & 0x08)
    return STATUS_COLLISION;

  if (backData && backLen && checkCRC) {
    if (*backLen == 1 && _validBits == 4)
      return STATUS_MIFARE_NACK;
    if (*backLen < 2 || _validBits != 0)
      return STATUS_CRC_WRONG;

    uint8_t controlBuffer[2];
    StatusCode status =
        PCD_CalculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
    if (status != STATUS_OK)
      return status;

    if ((backData[*backLen - 2] != controlBuffer[0]) ||
        (backData[*backLen - 1] != controlBuffer[1])) {
      return STATUS_CRC_WRONG;
    }
  }

  return STATUS_OK;
}

MFRC522::StatusCode MFRC522::PICC_RequestA(uint8_t *bufferATQA,
                                           uint8_t *bufferSize) {
  return PICC_REQA_or_WUPA(PICC_CMD_REQA, bufferATQA, bufferSize);
}

MFRC522::StatusCode MFRC522::PICC_WakeupA(uint8_t *bufferATQA,
                                          uint8_t *bufferSize) {
  return PICC_REQA_or_WUPA(PICC_CMD_WUPA, bufferATQA, bufferSize);
}

MFRC522::StatusCode MFRC522::PICC_REQA_or_WUPA(uint8_t command,
                                               uint8_t *bufferATQA,
                                               uint8_t *bufferSize) {
  uint8_t validBits;
  StatusCode status;

  if (bufferATQA == NULL || *bufferSize < 2)
    return STATUS_NO_ROOM;

  PCD_ClearRegisterBitMask(CollReg, 0x80);
  validBits = 7;
  status = PCD_TransceiveData(&command, 1, bufferATQA, bufferSize, &validBits);

  if (status != STATUS_OK)
    return status;
  if (*bufferSize != 2 || validBits != 0)
    return STATUS_ERROR;

  return STATUS_OK;
}

MFRC522::StatusCode MFRC522::PICC_Select(Uid *uid, uint8_t validBits) {
  bool uidComplete;
  bool selectDone;
  bool useCascadeTag;
  uint8_t cascadeLevel = 1;
  StatusCode result;
  uint8_t count;
  uint8_t index;
  uint8_t uidIndex;
  int8_t currentLevelKnownBits;
  uint8_t buffer[9];
  uint8_t bufferUsed;
  uint8_t rxAlign;
  uint8_t txLastBits;
  uint8_t *responseBuffer;
  uint8_t responseLength;

  if (validBits > 80)
    return STATUS_INVALID;

  PCD_ClearRegisterBitMask(CollReg, 0x80);

  uidComplete = false;
  while (!uidComplete) {
    switch (cascadeLevel) {
    case 1:
      buffer[0] = PICC_CMD_SEL_CL1;
      uidIndex = 0;
      useCascadeTag = validBits && uid->size > 4;
      break;
    case 2:
      buffer[0] = PICC_CMD_SEL_CL2;
      uidIndex = 3;
      useCascadeTag = validBits && uid->size > 7;
      break;
    case 3:
      buffer[0] = PICC_CMD_SEL_CL3;
      uidIndex = 6;
      useCascadeTag = false;
      break;
    default:
      return STATUS_INTERNAL_ERROR;
    }

    currentLevelKnownBits = validBits - (8 * uidIndex);
    if (currentLevelKnownBits < 0)
      currentLevelKnownBits = 0;

    index = 2;
    if (useCascadeTag) {
      buffer[index++] = PICC_CMD_CT;
    }

    uint8_t bytesToCopy =
        currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0);
    if (bytesToCopy) {
      uint8_t maxBytes = useCascadeTag ? 3 : 4;
      if (bytesToCopy > maxBytes)
        bytesToCopy = maxBytes;
      for (count = 0; count < bytesToCopy; count++) {
        buffer[index++] = uid->uidByte[uidIndex + count];
      }
    }

    if (useCascadeTag) {
      currentLevelKnownBits += 8;
    }

    selectDone = false;
    while (!selectDone) {
      if (currentLevelKnownBits >= 32) {
        buffer[1] = 0x70;
        buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
        result = PCD_CalculateCRC(buffer, 7, &buffer[7]);
        if (result != STATUS_OK)
          return result;

        txLastBits = 0;
        bufferUsed = 9;
        responseBuffer = &buffer[6];
        responseLength = 3;
      } else {
        txLastBits = currentLevelKnownBits % 8;
        count = currentLevelKnownBits / 8;
        index = 2 + count;
        buffer[1] = (index << 4) + txLastBits;
        bufferUsed = index + (txLastBits ? 1 : 0);
        responseBuffer = &buffer[index];
        responseLength = sizeof(buffer) - index;
      }

      rxAlign = txLastBits;
      PCD_WriteRegister(BitFramingReg, (rxAlign << 4) + txLastBits);

      result = PCD_TransceiveData(buffer, bufferUsed, responseBuffer,
                                  &responseLength, &txLastBits, rxAlign);

      if (result == STATUS_COLLISION) {
        uint8_t valueOfCollReg = PCD_ReadRegister(CollReg);
        if (valueOfCollReg & 0x20)
          return STATUS_COLLISION;

        uint8_t collisionPos = valueOfCollReg & 0x1F;
        if (collisionPos == 0)
          collisionPos = 32;
        if (collisionPos <= currentLevelKnownBits)
          return STATUS_INTERNAL_ERROR;

        currentLevelKnownBits = collisionPos;
        count = (currentLevelKnownBits - 1) % 8;
        index = 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0);
        buffer[index] |= (1 << count);
      } else if (result != STATUS_OK) {
        return result;
      } else {
        if (currentLevelKnownBits >= 32) {
          selectDone = true;
        } else {
          currentLevelKnownBits = 32;
        }
      }
    }

    index = (buffer[2] == PICC_CMD_CT) ? 3 : 2;
    bytesToCopy = (buffer[2] == PICC_CMD_CT) ? 3 : 4;
    for (count = 0; count < bytesToCopy; count++) {
      uid->uidByte[uidIndex + count] = buffer[index++];
    }

    if (responseLength != 3 || txLastBits != 0)
      return STATUS_ERROR;

    result = PCD_CalculateCRC(responseBuffer, 1, &buffer[2]);
    if (result != STATUS_OK)
      return result;

    if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2])) {
      return STATUS_CRC_WRONG;
    }

    if (responseBuffer[0] & 0x04) {
      cascadeLevel++;
    } else {
      uidComplete = true;
      uid->sak = responseBuffer[0];
    }
  }

  uid->size = 3 * cascadeLevel + 1;

  return STATUS_OK;
}

MFRC522::StatusCode MFRC522::PICC_HaltA() {
  StatusCode result;
  uint8_t buffer[4];

  buffer[0] = PICC_CMD_HLTA;
  buffer[1] = 0;
  result = PCD_CalculateCRC(buffer, 2, &buffer[2]);
  if (result != STATUS_OK)
    return result;

  result = PCD_TransceiveData(buffer, sizeof(buffer), NULL, 0);
  if (result == STATUS_TIMEOUT)
    return STATUS_OK;
  if (result == STATUS_OK)
    return STATUS_ERROR;

  return result;
}

MFRC522::StatusCode MFRC522::PCD_Authenticate(uint8_t command,
                                              uint8_t blockAddr,
                                              MIFARE_Key *key, Uid *uid) {
  uint8_t waitIRq = 0x10;
  uint8_t sendData[12];

  sendData[0] = command;
  sendData[1] = blockAddr;
  for (uint8_t i = 0; i < 6; i++) {
    sendData[2 + i] = key->keyByte[i];
  }
  for (uint8_t i = 0; i < 4; i++) {
    sendData[8 + i] = uid->uidByte[i];
  }

  return PCD_CommunicateWithPICC(PCD_MFAuthent, waitIRq, &sendData[0],
                                 sizeof(sendData));
}

void MFRC522::PCD_StopCrypto1() { PCD_ClearRegisterBitMask(Status2Reg, 0x08); }

MFRC522::StatusCode MFRC522::MIFARE_Read(uint8_t blockAddr, uint8_t *buffer,
                                         uint8_t *bufferSize) {
  StatusCode result;

  if (buffer == NULL || *bufferSize < 18)
    return STATUS_NO_ROOM;

  buffer[0] = PICC_CMD_MF_READ;
  buffer[1] = blockAddr;
  result = PCD_CalculateCRC(buffer, 2, &buffer[2]);
  if (result != STATUS_OK)
    return result;

  return PCD_TransceiveData(buffer, 4, buffer, bufferSize, NULL, 0, true);
}

MFRC522::StatusCode MFRC522::MIFARE_Write(uint8_t blockAddr, uint8_t *buffer,
                                          uint8_t bufferSize) {
  StatusCode result;

  if (buffer == NULL || bufferSize < 16)
    return STATUS_INVALID;

  uint8_t cmdBuffer[2];
  cmdBuffer[0] = PICC_CMD_MF_WRITE;
  cmdBuffer[1] = blockAddr;
  result = PCD_MIFARE_Transceive(cmdBuffer, 2);
  if (result != STATUS_OK)
    return result;

  result = PCD_MIFARE_Transceive(buffer, bufferSize);
  return result;
}

MFRC522::StatusCode MFRC522::PCD_MIFARE_Transceive(uint8_t *sendData,
                                                   uint8_t sendLen,
                                                   bool acceptTimeout) {
  StatusCode result;
  uint8_t cmdBuffer[18];

  if (sendData == NULL || sendLen > 16)
    return STATUS_INVALID;

  memcpy(cmdBuffer, sendData, sendLen);
  result = PCD_CalculateCRC(cmdBuffer, sendLen, &cmdBuffer[sendLen]);
  if (result != STATUS_OK)
    return result;

  sendLen += 2;
  uint8_t waitIRq = 0x30;
  uint8_t cmdBufferSize = sizeof(cmdBuffer);
  uint8_t validBits = 0;
  result = PCD_CommunicateWithPICC(PCD_Transceive, waitIRq, cmdBuffer, sendLen,
                                   cmdBuffer, &cmdBufferSize, &validBits);

  if (acceptTimeout && result == STATUS_TIMEOUT)
    return STATUS_OK;
  if (result != STATUS_OK)
    return result;

  if (cmdBufferSize != 1 || validBits != 4)
    return STATUS_ERROR;
  if (cmdBuffer[0] != 0x0A)
    return STATUS_MIFARE_NACK;

  return STATUS_OK;
}

bool MFRC522::PICC_IsNewCardPresent() {
  uint8_t bufferATQA[2];
  uint8_t bufferSize = sizeof(bufferATQA);

  PCD_WriteRegister(TxModeReg, 0x00);
  PCD_WriteRegister(RxModeReg, 0x00);
  PCD_WriteRegister(ModWidthReg, 0x26);

  StatusCode result = PICC_RequestA(bufferATQA, &bufferSize);
  return (result == STATUS_OK || result == STATUS_COLLISION);
}

bool MFRC522::PICC_ReadCardSerial() {
  StatusCode result = PICC_Select(&uid);
  return (result == STATUS_OK);
}

uint8_t MFRC522::PICC_GetType(uint8_t sak) {
  sak &= 0x7F;
  switch (sak) {
  case 0x04:
    return PICC_TYPE_NOT_COMPLETE;
  case 0x09:
    return PICC_TYPE_MIFARE_MINI;
  case 0x08:
    return PICC_TYPE_MIFARE_1K;
  case 0x18:
    return PICC_TYPE_MIFARE_4K;
  case 0x00:
    return PICC_TYPE_MIFARE_UL;
  case 0x10:
  case 0x11:
    return PICC_TYPE_MIFARE_PLUS;
  case 0x01:
    return PICC_TYPE_TNP3XXX;
  case 0x20:
    return PICC_TYPE_ISO_14443_4;
  case 0x40:
    return PICC_TYPE_ISO_18092;
  default:
    return PICC_TYPE_UNKNOWN;
  }
}

const char *MFRC522::PICC_GetTypeName(uint8_t piccType) {
  switch (piccType) {
  case PICC_TYPE_ISO_14443_4:
    return "PICC compliant with ISO/IEC 14443-4";
  case PICC_TYPE_ISO_18092:
    return "PICC compliant with ISO/IEC 18092 (NFC)";
  case PICC_TYPE_MIFARE_MINI:
    return "MIFARE Mini, 320 bytes";
  case PICC_TYPE_MIFARE_1K:
    return "MIFARE 1KB";
  case PICC_TYPE_MIFARE_4K:
    return "MIFARE 4KB";
  case PICC_TYPE_MIFARE_UL:
    return "MIFARE Ultralight or Ultralight C";
  case PICC_TYPE_MIFARE_PLUS:
    return "MIFARE Plus";
  case PICC_TYPE_TNP3XXX:
    return "MIFARE TNP3XXX";
  case PICC_TYPE_NOT_COMPLETE:
    return "SAK indicates UID is not complete";
  case PICC_TYPE_UNKNOWN:
  default:
    return "Unknown type";
  }
}

const char *MFRC522::GetStatusCodeName(StatusCode code) {
  switch (code) {
  case STATUS_OK:
    return "Success";
  case STATUS_ERROR:
    return "Error in communication";
  case STATUS_COLLISION:
    return "Collision detected";
  case STATUS_TIMEOUT:
    return "Timeout in communication";
  case STATUS_NO_ROOM:
    return "A buffer is not big enough";
  case STATUS_INTERNAL_ERROR:
    return "Internal error in the code";
  case STATUS_INVALID:
    return "Invalid argument";
  case STATUS_CRC_WRONG:
    return "CRC check failed";
  case STATUS_MIFARE_NACK:
    return "MIFARE NACK";
  default:
    return "Unknown error";
  }
}

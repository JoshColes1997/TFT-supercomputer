#ifndef MFRC522_H
#define MFRC522_H

#include "mbed.h"

class MFRC522 {
public:
    enum PCD_Register {
        CommandReg            = 0x01,
        ComIEnReg             = 0x02,
        DivIEnReg             = 0x03,
        ComIrqReg             = 0x04,
        DivIrqReg             = 0x05,
        ErrorReg              = 0x06,
        Status1Reg            = 0x07,
        Status2Reg            = 0x08,
        FIFODataReg           = 0x09,
        FIFOLevelReg          = 0x0A,
        WaterLevelReg         = 0x0B,
        ControlReg            = 0x0C,
        BitFramingReg         = 0x0D,
        CollReg               = 0x0E,
        ModeReg               = 0x11,
        TxModeReg             = 0x12,
        RxModeReg             = 0x13,
        TxControlReg          = 0x14,
        TxASKReg              = 0x15,
        TxSelReg              = 0x16,
        RxSelReg              = 0x17,
        RxThresholdReg        = 0x18,
        DemodReg              = 0x19,
        MfTxReg               = 0x1C,
        MfRxReg               = 0x1D,
        SerialSpeedReg        = 0x1F,
        CRCResultRegH         = 0x21,
        CRCResultRegL         = 0x22,
        ModWidthReg           = 0x24,
        RFCfgReg              = 0x26,
        GsNReg                = 0x27,
        CWGsPReg              = 0x28,
        ModGsPReg             = 0x29,
        TModeReg              = 0x2A,
        TPrescalerReg         = 0x2B,
        TReloadRegH           = 0x2C,
        TReloadRegL           = 0x2D,
        TCounterValueRegH     = 0x2E,
        TCounterValueRegL     = 0x2F,
        TestSel1Reg           = 0x31,
        TestSel2Reg           = 0x32,
        TestPinEnReg          = 0x33,
        TestPinValueReg       = 0x34,
        TestBusReg            = 0x35,
        AutoTestReg           = 0x36,
        VersionReg            = 0x37,
        AnalogTestReg         = 0x38,
        TestDAC1Reg           = 0x39,
        TestDAC2Reg           = 0x3A,
        TestADCReg            = 0x3B
    };

    enum PCD_Command {
        PCD_Idle              = 0x00,
        PCD_Mem               = 0x01,
        PCD_GenerateRandomID  = 0x02,
        PCD_CalcCRC           = 0x03,
        PCD_Transmit          = 0x04,
        PCD_NoCmdChange       = 0x07,
        PCD_Receive           = 0x08,
        PCD_Transceive        = 0x0C,
        PCD_MFAuthent         = 0x0E,
        PCD_SoftReset         = 0x0F
    };

    enum PICC_Command {
        PICC_CMD_REQA         = 0x26,
        PICC_CMD_WUPA         = 0x52,
        PICC_CMD_CT           = 0x88,
        PICC_CMD_SEL_CL1      = 0x93,
        PICC_CMD_SEL_CL2      = 0x95,
        PICC_CMD_SEL_CL3      = 0x97,
        PICC_CMD_HLTA         = 0x50,
        PICC_CMD_MF_AUTH_KEY_A = 0x60,
        PICC_CMD_MF_AUTH_KEY_B = 0x61,
        PICC_CMD_MF_READ      = 0x30,
        PICC_CMD_MF_WRITE     = 0xA0,
        PICC_CMD_MF_DECREMENT = 0xC0,
        PICC_CMD_MF_INCREMENT = 0xC1,
        PICC_CMD_MF_RESTORE   = 0xC2,
        PICC_CMD_MF_TRANSFER  = 0xB0,
        PICC_CMD_UL_WRITE     = 0xA2
    };

    enum StatusCode {
        STATUS_OK             = 1,
        STATUS_ERROR          = 2,
        STATUS_COLLISION      = 3,
        STATUS_TIMEOUT        = 4,
        STATUS_NO_ROOM        = 5,
        STATUS_INTERNAL_ERROR = 6,
        STATUS_INVALID        = 7,
        STATUS_CRC_WRONG      = 8,
        STATUS_MIFARE_NACK    = 9
    };

    struct Uid {
        uint8_t size;
        uint8_t uidByte[10];
        uint8_t sak;
    };

    struct MIFARE_Key {
        uint8_t keyByte[6];
    };

    MFRC522(PinName mosi, PinName miso, PinName sclk, PinName cs, PinName reset);
    
    void PCD_Init();
    void PCD_Reset();
    void PCD_AntennaOn();
    void PCD_AntennaOff();
    uint8_t PCD_GetAntennaGain();
    void PCD_SetAntennaGain(uint8_t mask);
    bool PCD_PerformSelfTest();
    
    void PCD_WriteRegister(uint8_t reg, uint8_t value);
    void PCD_WriteRegister(uint8_t reg, uint8_t count, uint8_t *values);
    uint8_t PCD_ReadRegister(uint8_t reg);
    void PCD_ReadRegister(uint8_t reg, uint8_t count, uint8_t *values, uint8_t rxAlign = 0);
    void PCD_SetRegisterBitMask(uint8_t reg, uint8_t mask);
    void PCD_ClearRegisterBitMask(uint8_t reg, uint8_t mask);
    StatusCode PCD_CalculateCRC(uint8_t *data, uint8_t length, uint8_t *result);
    
    StatusCode PCD_TransceiveData(uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits = NULL, uint8_t rxAlign = 0, bool checkCRC = false);
    StatusCode PCD_CommunicateWithPICC(uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen, uint8_t *backData = NULL, uint8_t *backLen = NULL, uint8_t *validBits = NULL, uint8_t rxAlign = 0, bool checkCRC = false);
    StatusCode PICC_RequestA(uint8_t *bufferATQA, uint8_t *bufferSize);
    StatusCode PICC_WakeupA(uint8_t *bufferATQA, uint8_t *bufferSize);
    StatusCode PICC_REQA_or_WUPA(uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize);
    StatusCode PICC_Select(Uid *uid, uint8_t validBits = 0);
    StatusCode PICC_HaltA();
    
    StatusCode PCD_Authenticate(uint8_t command, uint8_t blockAddr, MIFARE_Key *key, Uid *uid);
    void PCD_StopCrypto1();
    StatusCode MIFARE_Read(uint8_t blockAddr, uint8_t *buffer, uint8_t *bufferSize);
    StatusCode MIFARE_Write(uint8_t blockAddr, uint8_t *buffer, uint8_t bufferSize);
    StatusCode MIFARE_Decrement(uint8_t blockAddr, int32_t delta);
    StatusCode MIFARE_Increment(uint8_t blockAddr, int32_t delta);
    StatusCode MIFARE_Restore(uint8_t blockAddr);
    StatusCode MIFARE_Transfer(uint8_t blockAddr);
    StatusCode MIFARE_UltralightWrite(uint8_t page, uint8_t *buffer, uint8_t bufferSize);
    StatusCode MIFARE_GetValue(uint8_t blockAddr, int32_t *value);
    StatusCode MIFARE_SetValue(uint8_t blockAddr, int32_t value);
    StatusCode PCD_MIFARE_Transceive(uint8_t *sendData, uint8_t sendLen, bool acceptTimeout = false);
    
    bool PICC_IsNewCardPresent();
    bool PICC_ReadCardSerial();
    
    static uint8_t PICC_GetType(uint8_t sak);
    static const char* PICC_GetTypeName(uint8_t type);
    static const char* GetStatusCodeName(StatusCode code);
    
    void PICC_DumpToSerial(Uid *uid);
    void PICC_DumpDetailsToSerial(Uid *uid);
    void PICC_DumpMifareClassicToSerial(Uid *uid, uint8_t piccType, MIFARE_Key *key);
    void PICC_DumpMifareClassicSectorToSerial(Uid *uid, MIFARE_Key *key, uint8_t sector);
    void PIFARE_UltralightDumpToSerial();
    
    void MIFARE_SetAccessBits(uint8_t *accessBitBuffer, uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3);
    bool MIFARE_OpenUidBackdoor(bool logErrors);
    bool MIFARE_SetUid(uint8_t *newUid, uint8_t uidSize, bool logErrors);
    bool MIFARE_UnbrickUidSector(bool logErrors);
    
    Uid uid;

private:
    SPI _spi;
    DigitalOut _cs;
    DigitalOut _reset;
    
    static const uint8_t FIFO_SIZE = 64;
};

enum PICC_Type {
    PICC_TYPE_UNKNOWN       = 0,
    PICC_TYPE_ISO_14443_4   = 1,
    PICC_TYPE_ISO_18092     = 2,
    PICC_TYPE_MIFARE_MINI   = 3,
    PICC_TYPE_MIFARE_1K     = 4,
    PICC_TYPE_MIFARE_4K     = 5,
    PICC_TYPE_MIFARE_UL     = 6,
    PICC_TYPE_MIFARE_PLUS   = 7,
    PICC_TYPE_TNP3XXX       = 8,
    PICC_TYPE_NOT_COMPLETE  = 255
};

#endif

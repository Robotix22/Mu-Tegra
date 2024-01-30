#ifndef _TEGRA_GPIO_H_
#define _TEGRA_GPIO_H_

//
// Global GUID for the Tegra Gpio Protocol
//
#define TEGRA_GPIO_PROTOCOL_GUID { 0xf5a3b253, 0x587d, 0x4ee5, { 0xa3, 0x4a, 0xf4, 0x86, 0x63, 0x1d, 0x9f, 0x2e } }

//
// Declare forward reference to the Tegra Gpio Protocol
//
typedef struct _TEGRA_GPIO_PROTOCOL TEGRA_GPIO_PROTOCOL;

//
// Tegra Gpio Ports
//
#define TEGRA_GPIO_PORT_A  0
#define TEGRA_GPIO_PORT_B  1
#define TEGRA_GPIO_PORT_C  2
#define TEGRA_GPIO_PORT_D  3
#define TEGRA_GPIO_PORT_E  4
#define TEGRA_GPIO_PORT_F  5
#define TEGRA_GPIO_PORT_G  6
#define TEGRA_GPIO_PORT_H  7
#define TEGRA_GPIO_PORT_I  8
#define TEGRA_GPIO_PORT_J  9
#define TEGRA_GPIO_PORT_K  10
#define TEGRA_GPIO_PORT_L  11
#define TEGRA_GPIO_PORT_M  12
#define TEGRA_GPIO_PORT_N  13
#define TEGRA_GPIO_PORT_O  14
#define TEGRA_GPIO_PORT_P  15
#define TEGRA_GPIO_PORT_Q  16
#define TEGRA_GPIO_PORT_R  17
#define TEGRA_GPIO_PORT_S  18
#define TEGRA_GPIO_PORT_T  19
#define TEGRA_GPIO_PORT_U  20
#define TEGRA_GPIO_PORT_V  21
#define TEGRA_GPIO_PORT_W  22
#define TEGRA_GPIO_PORT_X  23
#define TEGRA_GPIO_PORT_Y  24
#define TEGRA_GPIO_PORT_Z  25
#define TEGRA_GPIO_PORT_AA 26
#define TEGRA_GPIO_PORT_BB 27
#define TEGRA_GPIO_PORT_CC 28
#define TEGRA_GPIO_PORT_DD 29
#define TEGRA_GPIO_PORT_EE 30
#define TEGRA_GPIO_PORT_FF 31

#define TEGRA_GPIO(port, offset) ((TEGRA_GPIO_PORT_##port * 8) + offset)


/**
  This Function Gets the Current Configuration of the Defined GPIO.
  
  @param  Gpio        The Gpio.
  
  @return Type        The Configuration Type.
**/
typedef
INT32
(EFIAPI *EFI_GET_CONFIG)(
  UINTN Gpio
  );

/**
  This Function Sets the Defined Configuration to the defined GPIO.

  @param  Gpio        The Gpio.
  @param  Type        The Configuration.

  @return EFI_SUCCESS The Configuration was set Successfully.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_CONFIG)(
  UINTN Gpio,
  INT32 Type
  );

/**
  This Function Gets the Current Direction of the Defined GPIO.

  @param  Gpio        The Gpio.

  @return Direction   The Direction.
**/
typedef
INT32
(EFIAPI *EFI_GET_DIRECTION)(
  UINTN Gpio
  );

/**
  This Function Sets the Defined Direction to the Defined GPIO.

  @param  Gpio        The Gpio.
  @param  Direction   The Direction.

  @return EFI_SUCCESS The Direction was set Successfully.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_DIRECTION)(
  UINTN Gpio,
  INT32 Direction
  );

/**
  This Function Sets the Level of the Defined GPIO.

  @param  Gpio        The Gpio.
  @param  High        The Level, TRUE if High, FALSE if Low.
**/
typedef
VOID
(EFIAPI *EFI_SET_LEVEL)(
  UINTN   Gpio,
  BOOLEAN High
  );

/**
  This Function gets the Current State of the defined GPIO.

  @param  Gpio        The Gpio.

  @return State       The Current State.
**/
typedef
INT32
(EFIAPI *EFI_GET_STATE)(
  UINTN Gpio
  );

//
// Define Protocol Functions
//
struct _TEGRA_GPIO_PROTOCOL {
  EFI_GET_CONFIG    GetConfig;
  EFI_SET_CONFIG    SetConfig;

  EFI_GET_DIRECTION GetDirection;
  EFI_SET_DIRECTION SetDirection;

  EFI_SET_LEVEL     SetLevel;

  EFI_GET_STATE     GetState;
};

extern EFI_GUID gTegraGpioProtocolGuid;

#endif /* _TEGRA_GPIO_H_ */

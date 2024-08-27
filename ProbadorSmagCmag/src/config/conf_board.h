/**
 * \file
 *
 * \brief User board configuration template
 *
 * Copyright (c) 2013-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H
/*
* Para energizar y testear que haya tension en la placa.
*/
#define POWER				PIN_PA09
#define POWER_FEED			PIN_PA10
//#define BOTON_SERIALIZAR	PIN_PA11

/*
* Son las entrada y salida de MPHX
*/
#define MPXH_ENT			PIN_PA19
#define MPXH_SAL			PIN_PA18

/*
* Para temporizar la lectura de la vuelta del programa principal
*/
#define DUTY_INT			PIN_PA02
#define DUTY_PAL			PIN_PA03


//#define W4038_UART_MODULE              SERCOM5 // Manejo de protocolos I2C SPI UART
//#define W4038_UART_SERCOM_MUX_SETTING  USART_RX_3_TX_2_XCK_3
//#define W4038_UART_SERCOM_PINMUX_PAD0  PINMUX_UNUSED
//#define W4038_UART_SERCOM_PINMUX_PAD1  PINMUX_UNUSED
//#define W4038_UART_SERCOM_PINMUX_PAD2  PINMUX_PA20C_SERCOM5_PAD2
//#define W4038_UART_SERCOM_PINMUX_PAD3  PINMUX_PA21C_SERCOM5_PAD3
//#define W4038_UART_SERCOM_DMAC_ID_TX   SERCOM5_DMAC_ID_TX
//#define W4038_UART_SERCOM_DMAC_ID_RX   SERCOM5_DMAC_ID_RX

//#define PC_UART_MODULE              SERCOM2
//#define PC_UART_SERCOM_MUX_SETTING  USART_RX_0_TX_2_XCK_3
//#define PC_UART_SERCOM_PINMUX_PAD0  PINMUX_PA12C_SERCOM2_PAD0
//#define PC_UART_SERCOM_PINMUX_PAD1  PINMUX_UNUSED
//#define PC_UART_SERCOM_PINMUX_PAD2  PINMUX_PA14C_SERCOM2_PAD2
//#define PC_UART_SERCOM_PINMUX_PAD3  PINMUX_UNUSED
//#define PC_UART_SERCOM_DMAC_ID_TX   SERCOM2_DMAC_ID_TX
//#define PC_UART_SERCOM_DMAC_ID_RX   SERCOM2_DMAC_ID_RX

/*
* TODO: Definir pines para comandar electroimán y simular el switch
*/
#define IMAN_ENT	PIN_PA17 // Verificar sacar R7 en hardware
#define IMAN_SAL	PIN_PA16 // Verificar sacar R30 en hardware
#define SMAG_ENT	PIN_PA08 // Board I/O 1
#define CMAG_ENT	PIN_PA09 // Board I/O 2
#endif // CONF_BOARD_H

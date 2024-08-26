# CAN_Bootloader
PCAN-USB Bootloader for STM32F103C8T6
### Project Description
Welcome to the PCAN-USB Bootloader project for the STM32F103C8T6 microcontroller! This project provides a complete solution for flashing firmware onto STM32F103C8T6 boards using a PCAN-USB interface. It features a bootloader that facilitates firmware updates via the CAN (Controller Area Network) protocol, leveraging the PCAN-USB adapter.

### Features
Bootloader Implementation: A custom bootloader designed to receive and flash firmware updates over CAN using the PCAN-USB interface.
Firmware Upload: Supports seamless firmware upload to STM32F103C8T6, allowing for easy updates and debugging.
PCAN-USB Compatibility: Utilizes the PCAN-USB adapter for CAN communication, providing a reliable and straightforward method for firmware flashing.
STM32CubeMX Integration: Configured to work with STM32CubeMX, ensuring compatibility with STM32 development tools.
Example Firmware: Includes example firmware to demonstrate the bootloader's functionality and provide a starting point for custom applications.
Requirements
Hardware:

### STM32F103C8T6 microcontroller board.
PCAN-USB adapter for CAN communication.
A computer with the PCAN-View or PCAN-Tools software installed.
### Software:

STM32CubeMX for configuring and generating STM32 initialization code.
STM32CubeIDE or another compatible IDE for developing and building the firmware.
PCAN-USB driver and software for CAN communication.
### Getting Started
Hardware Setup:

Connect the STM32F103C8T6 board to your PC using the PCAN-USB adapter.
Ensure proper power supply and CAN bus connections.
###  Software Installation:

Install the PCAN-USB drivers and software from the PEAK-System website.
Install STM32CubeMX and STM32CubeIDE from STMicroelectronics.
###  Bootloader Configuration:

Use STM32CubeMX to configure the bootloader settings and generate initialization code.
Flash the bootloader onto the STM32F103C8T6 board using your preferred programming tool.
Firmware Upload:

Prepare your firmware and use the PCAN-USB adapter to upload it to the STM32F103C8T6 via the bootloader.
Testing:

Verify the firmware functionality and ensure that the bootloader operates correctly.
### Documentation
Bootloader Source Code: Detailed implementation of the bootloader for handling CAN communication and firmware flashing.

User Guide: Step-by-step instructions for setting up the bootloader, uploading firmware, and troubleshooting common issues.
Contributing
Contributions are welcome! Please feel free to open issues or submit pull requests. If you have suggestions for improvements or new features, we would love to hear from you.

License
This project is licensed under the MIT License. See the LICENSE file for more details.

Thank you for checking out the PCAN-USB Bootloader project! We hope it helps streamline your development process for STM32F103C8T6 microcontrollers. If you have any questions or need support, don't hesitate to reach out.

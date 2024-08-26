import can
import sys
import time
import argparse
import logging
from enum import Enum

logging.basicConfig(format='%(levelname)-8s [%(filename)s:%(lineno)d] %(message)s', level=logging.DEBUG)
logger = logging.getLogger(__name__)
logger.debug('Hello!')

STD_ID_BITS_NUM = 11 # number of identifier's bits in standard CAN frame 
ID_BITS_NUM = 8 # number of bits used by loader to determine a single STM device
LOADER_ID = 0x88 # id of our computer loader, the same number is hardcoded in a bootloader app flashed on an STM device

class BootOptions(Enum):
    JUMP_TO_APP = 1
    ERASE = 2
    FLASH = 3
    TEST_LOADER = 4
    HANDSHAKE = 5

class Bootloader:
    def __init__(self, sys_argv=None):
        logger.debug('Initializing')
        if sys_argv is None:
            sys_argv = sys.argv[1:]

        parser = argparse.ArgumentParser(description='Communicate with STM32F1 CAN bootloader to erase and flash it.',
                                         epilog='More info about interfaces and channels could be found here: https://python-can.readthedocs.io')
        parser.add_argument('--handshake', action='store_true', help='When an app is already flashed on your device, use this flag to set the loader in a waiting state, then reset the device.')
        parser.add_argument('--erase', action='store_true', help='Erase flash memory of a device, not including CAN bootloader.')
        parser.add_argument('--flash', action='store_true', help='Flash a device with a specified program in the --file field.')
        parser.add_argument('--test_loader', action='store_true', help='Check if connected with the loader on STM.')
        parser.add_argument('--jump_to_app', action='store_true', help='Command a device to start executing a flashed program.')

        parser.add_argument('--file', '-f', type=str, help='Path of the binary file that has to be loaded.')
        parser.add_argument('--deviceId', '-n', type=int, default=0x88, help='ID number of STM device to be loaded.')
        parser.add_argument('--interface', '-i', type=str, default='pcan', help='Type of interface used to communicate on CAN bus by your computer.')
        parser.add_argument('--channel', '-c', type=str, default='PCAN_USBBUS1', help='Interface identifier in your system, e.g., PCAN_USBBUS1.')
        parser.add_argument('--speed', '-s', type=int, default=500000, help='CAN bus speed.')

        self.args = parser.parse_args(sys_argv)
        self.bus = can.interface.Bus(interface=self.args.interface, channel=self.args.channel, bitrate=self.args.speed)

        if self.args.file:
            self.file = open(self.args.file, 'rb')
        self.deviceId = (self.args.deviceId << (STD_ID_BITS_NUM - ID_BITS_NUM))

    def mass_erase(self):
        msg = can.Message(is_extended_id=False, arbitration_id=BootOptions.ERASE.value | self.deviceId)
        self.bus.send(msg)
        time.sleep(5.) # Ensure enough time to erase all data

    def test_loader(self):
        msg = can.Message(is_extended_id=False, arbitration_id=BootOptions.TEST_LOADER.value | self.deviceId)
        self.bus.send(msg)
        msg = self.bus.recv(1.)
        if msg and (msg.arbitration_id == LOADER_ID):
            logger.info("Device's bootloader ready!")
        else:
            logger.info('Device not ready!')

    def flashIt(self):
        bytes = self.file.read(8)
        while bytes:
            msg = can.Message(is_extended_id=False, arbitration_id=BootOptions.FLASH.value | self.deviceId, data=bytes)
            self.bus.send(msg)
            logger.debug('Flashing bytes: {}'.format(bytes))
            time.sleep(0.01) # Temporary delay for debugging
            bytes = self.file.read(8)

    def jump_to_app(self):
        msg = can.Message(is_extended_id=False, arbitration_id=BootOptions.JUMP_TO_APP.value | self.deviceId)
        self.bus.send(msg)

    def main(self):
        logger.info('Starting app...')

        if self.args.handshake:
            logger.info('Waiting for a handshake from a device...')
            while True:
                msg = self.bus.recv()
                if msg.arbitration_id == LOADER_ID:
                    logger.info('Device found!')
                    msg = can.Message(is_extended_id=False, arbitration_id=BootOptions.HANDSHAKE.value | self.deviceId)
                    self.bus.send(msg)
                    break

        if self.args.erase:
            logger.info('Erasing...')
            self.mass_erase()

        if self.args.test_loader:
            logger.info('Testing loader...')
            self.test_loader()

        if self.args.flash:
            logger.info('Flashing...')
            self.flashIt()

        if self.args.jump_to_app:
            logger.info('Jumping to app')
            self.jump_to_app()

        logger.info('Mission accomplished, bye!')

if __name__ == '__main__':
    logger.debug('Running app')
    Bootloader().main()

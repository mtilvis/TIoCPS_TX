.. _TIoCPS_TX:

TIoCPS: TIoCPS Hunting Security Transmitter with Coded PHY
##########################################################

.. contents::
   :local:
   :depth: 2

Requirements
************

The sample supports the following development kits: nRF52840DK, nRF52840Dongle, nRF5340DK

.. table-from-sample-yaml::

.. include:: /includes/hci_rpmsg_overlay.txt

NOTE! To make the random number generation work set 
Random Number Generators:Non-random number generator at kconfig.

Overview
********

The sample demonstrates a basic Bluetooth® Low Energy Peripheral role functionality that exposes the Broadcast with LE Coded PHY support, 
which is not available in Zephyr Bluetooth LE Controller (See :ref:`ug_ble_controller` for more information).

Code broadcasts Hunting Security information with ID and location as described in TIoCPS D6.3


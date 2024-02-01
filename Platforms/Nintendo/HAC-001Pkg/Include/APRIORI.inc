APRIORI DXE {
    # First, install the PCD driver and call DxeInit
    INF MdeModulePkg/Core/Dxe/DxeMain.inf
    INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

    INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
    INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
    INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

    # Next, install cpu protocol and enable the interrupt controller
    INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
    INF ArmPkg/Drivers/ArmGic/ArmGicDxe.inf

    # Core Drivers
    INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
    INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf
    INF ArmPkg/Drivers/ArmPsciMpServicesDxe/ArmPsciMpServicesDxe.inf

    # NOTE: Clocks also might be needed earlier than this
    INF NvidiaPkg/GPLDrivers/TegraClockDxe/TegraClockDxe.inf
    INF NvidiaPkg/GPLDrivers/TegraGpioDxe/TegraGpioDxe.inf

    INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
    INF MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
    INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
    INF EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
    INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
    INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
    INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
    INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
    INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
    INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

    INF NvidiaPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf

    INF EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
    INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
    INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
    INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf

    INF NvidiaPkg/Drivers/KeypadDeviceDxe/KeypadDeviceDxe.inf
    INF NvidiaPkg/Drivers/KeypadDxe/KeypadDxe.inf
}

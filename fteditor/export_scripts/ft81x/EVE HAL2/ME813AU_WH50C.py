import os,shutil, subprocess, sys, re, imp

supportedPlatforms = [2064, 2065, 2066, 2067]
exportModuleName = "export_ftdi_eve_hal2"
deviceModuleName = "ME813AU_WH50C"

missingFileMessage = 'Unable to locate the required export file in the installation directory.'
wrongPlatformMessage = 'The project can not be exported to the selected platform.  The selected platform is incompatible with the current project device type.'

def displayName():
    return deviceModuleName

def run(name, document, ram):
    device_type = document["project"]["device"]
    if device_type not in supportedPlatforms:
        return wrongPlatformMessage
    else:
        exportScriptPath = os.path.abspath(__file__).rsplit("export_scripts", 1)
        if len(exportScriptPath):
            exportScriptName = exportScriptPath[0] + exportModuleName + ".py"
            if os.path.exists(exportScriptName):
                exportScript = imp.load_source(exportModuleName, exportScriptName)
                return exportScript.run(name, document, ram, deviceModuleName)
            else:
                return missingFileMessage
        else:
            return missingFileMessage
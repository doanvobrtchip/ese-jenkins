import os,shutil, subprocess, sys, re, imp


def displayName():
    return "VM800BU43_50"


def run(name, document, ram):
    device_type = document["project"]["device"]
    if not device_type == 2048:
        return "The project can not be exported to the selected platform.  The selected platform is incompatible with the current project device type."
    else:
        exportScriptPath = os.path.abspath(__file__).split("export_scripts")
        exportScriptName = exportScriptPath[0] + "export_ftdi_eve_hal2.py"
        exportScript = imp.load_source("export_ftdi_eve_hal2", exportScriptName)
        return exportScript.run(name, document, ram, "VM800BU43_50")
/**
 * interactive_widgets.cpp
 * $Id$
 * \file interactive_widgets.cpp
 * \brief interactive_widgets.cpp
 * \date 2014-03-04 22:58GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include "interactive_widgets.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QSpinBox>

// Emulator includes

// Project includes
#include "main_window.h"
#include "dl_editor.h"
#include "properties_editor.h"
#include "constant_common.h"

namespace FTEDITOR {

int g_PropertiesWidgetCombineId = 99150;

} /* namespace FTEDITOR */

/* end of file */

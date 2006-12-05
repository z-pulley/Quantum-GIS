
import os, os.path
import copy, glob
import sipconfig
import PyQt4.pyqtconfig

##########################################################################
# SIP -> *.CPP + *.H

# The name of the SIP build file generated by SIP and used by the build
# system.
build_file_core = "core.sbf"
build_file_gui = "gui.sbf"

# Get the SIP configuration information.
config = PyQt4.pyqtconfig.Configuration()

# Get the extra SIP flags needed by the imported qt module.  Note that
# this normally only includes those flags (-x and -t) that relate to SIP's
# versioning system.
qt_sip_flags = config.pyqt_sip_flags

# directory where modules will be installed
mod_dir = os.path.join(config.default_mod_dir, "qgis")

# directory where sip files will be installed
sip_dir_core = os.path.join(config.default_sip_dir, "qgis/core")
sip_dir_gui = os.path.join(config.default_sip_dir, "qgis/gui")
  
# Run SIP to generate the code.
print "Parsing SIP files for 'core' library..."
os.system(" ".join([config.sip_bin, "-c", "core", "-b", "core/" + build_file_core, "-I", config.pyqt_sip_dir, qt_sip_flags, "core/core.sip"]))
print "Parsing SIP files for 'gui' library..."
os.system(" ".join([config.sip_bin, "-c", "gui", "-b", "gui/" + build_file_gui, "-I", config.pyqt_sip_dir, qt_sip_flags, "gui/gui.sip"]))


##########################################################################
# MAKEFILES

print "Creating makefiles..."

# We are going to install the SIP specification file for this module and
# its configuration module.
installs = []

# directories relative to core (gui) directories
installs.append([["../__init__.py", "../qgisconfig.py"], mod_dir])


installs_core = copy.copy(installs)
installs_gui = copy.copy(installs)

# install all sip files
sips_core = glob.glob("core/*.sip")
for sip in sips_core:
  installs_core.append([os.path.basename(sip), sip_dir_core])

sips_gui = glob.glob("gui/*.sip")
for sip in sips_gui:
  installs_gui.append([os.path.basename(sip), sip_dir_gui])


# Create the Makefile.  The QtModuleMakefile class provided by the
# pyqtconfig module takes care of all the extra preprocessor, compiler and
# linker flags needed by the Qt library.
makefile_core = sipconfig.ModuleMakefile(
        configuration=config,
        qt=["QtCore","QtGui","QtXml"],
        build_file=build_file_core,
        installs=installs_core,
        install_dir=mod_dir,
        dir="core")

makefile_gui = sipconfig.ModuleMakefile(
        configuration=config,
        qt=["QtCore","QtGui","QtXml"],
        build_file=build_file_gui,
        installs=installs_gui,
        install_dir=mod_dir,
        dir="gui")

# common settings for both core and gui libs
for mk in [ makefile_core, makefile_gui ]:
  mk.extra_libs = ["qgis_core"]
  mk.extra_lib_dirs = ["../../build/src/core"]  # TODO: make universal!
  mk.extra_include_dirs = ["../../src/core","../../src/core/raster","../../src/core/renderer","../../src/core/spatialindex","../../src/core/symbology","../.."]
  mk.extra_cxxflags = ["-DCORE_EXPORT="]

# more settings for gui lib
makefile_gui.extra_libs.append("qgis_gui")
makefile_gui.extra_lib_dirs.append("../../build/src/gui")
makefile_gui.extra_include_dirs.append("../../src/gui")
makefile_gui.extra_include_dirs.append("../../build/src/gui")
makefile_gui.extra_include_dirs.append("../../build/src/ui")
makefile_gui.extra_include_dirs.append("../../src/plugins") # because of qgisplugin.h TODO: sort out
makefile_gui.extra_cxxflags.append("-DGUI_EXPORT=")

# Generate the Makefile itself.
makefile_core.generate()
makefile_gui.generate()

##########################################################################
# QGIS CONFIG

print "Creating qgisconfig.py..."

# Now we create the configuration module.  This is done by merging a Python
# dictionary (whose values are normally determined dynamically) with a
# (static) template.
content = {
    # Publish where the SIP specifications for this module will be
    # installed.
    "qgis_sip_dir":    config.default_sip_dir,

    "qgis_mod_dir":    mod_dir,

    # Publish the set of SIP flags needed by this module.  As these are the
    # same flags needed by the qt module we could leave it out, but this
    # allows us to change the flags at a later date without breaking
    # scripts that import the configuration module.
    "qgis_sip_flags":  qt_sip_flags
}

# This creates the qgisconfig.py module from the qgisconfig.py.in
# template and the dictionary.
sipconfig.create_config_module("qgisconfig.py", "qgisconfig.py.in", content)

print "Done"


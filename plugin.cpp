#include <csignal>
#include <iostream>
#include <string>
#include <unistd.h>

#include <XPLMDisplay.h>
#include <XPLMPlugin.h>
#include <XPLMPlanes.h>
#include <XPLMProcessing.h>

#include <acfutils/acf_file.h>
#include <acfutils/crc64.h>
#include <acfutils/dr.h>
#include <acfutils/log.h>
#include <acfutils/helpers.h>
#include <acfutils/osrand.h>

#include "DatarefPublisher.hpp"

int interrupted = 0;

#define PLUGIN_NAME     "PanelClone"
#define PLUGIN_SIG      "com.sharedflight.PanelClone"
#define PLUGIN_DESCRIPTION  "Plugin for publishing state to synced cockpit devices"

static int      xpver = 0;
static char     plugindir[512] = { 0 };

static float NetworkingFlightLoopCallback(float elapsedMe, float elapsedSim, int counter, void * refcon);

static int
draw_cb(XPLMDrawingPhase phase, int before, void *refcon)
{
    return (1);
}

static void
log_dbg_string(const char *str)
{
    XPLMDebugString(str);
}

PLUGIN_API int
XPluginStart(char *name, char *sig, char *desc)
{
    char *p;
    uint64_t seed;
    int xplm_ver;
    XPLMHostApplicationID host_id;
    /*
     * libacfutils logging facility bootstrap, this must be one of
     * the first steps during init, to make sure we have the logMsg
     * and general error logging facilities available early.
     */
    log_init(log_dbg_string, "PanelManager");

    //set_plugin_id(XPLMGetMyID());
    //register_crash_handler();

    ASSERT(name != NULL);
    ASSERT(sig != NULL);
    ASSERT(desc != NULL);
    XPLMGetVersions(&xpver, &xplm_ver, &host_id);
    
    /*
     * Always use Unix-native paths on the Mac!
     */
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", 1);
    
    /*
     * Construct plugindir to point to our plugin's root directory.
     */
    XPLMGetPluginInfo(XPLMGetMyID(), NULL, plugindir, NULL, NULL);
    fix_pathsep(plugindir);
    /* cut off the trailing path component (our filename) */
    if ((p = strrchr(plugindir, DIRSEP)) != NULL)
        *p = '\0';
    /* cut off an optional '32' or '64' trailing component */
    if ((p = strrchr(plugindir, DIRSEP)) != NULL) {
        if (strcmp(p + 1, "64") == 0 || strcmp(p + 1, "32") == 0 ||
            strcmp(p + 1, "win_x64") == 0 ||
            strcmp(p + 1, "mac_x64") == 0 ||
            strcmp(p + 1, "lin_x64") == 0)
            *p = '\0';
    }
    /*
     * Initialize the CRC64 and PRNG machinery inside of libacfutils.
     */
    crc64_init();
    if (!osrand(&seed, sizeof (seed)))
        seed = microclock() + clock();
    crc64_srand(seed);
    
    strcpy(name, PLUGIN_NAME);
    strcpy(sig, PLUGIN_SIG);
    strcpy(desc, PLUGIN_DESCRIPTION);

    return (1);
errout:
    return (0);
}

PLUGIN_API void
XPluginStop(void)
{
    DatarefPublisher::getInstance().Finish();
    log_fini();
}

static inline const char *acf_find_prop(const acf_file_t *acf, const std::string property)
{
    return acf_prop_find(acf, property.c_str());
}

void new_aircraft_loaded()
{
    logMsg("[INFO] New aircraft load detected");
}

float NetworkingFlightLoopCallback(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
    DatarefPublisher::getInstance().GetFrame();
    
    //Return the interval we next want to be called in..
    return -1;
}


PLUGIN_API int
XPluginEnable(void)
{
    VERIFY(XPLMRegisterDrawCallback(draw_cb, xplm_Phase_Window, 1, NULL));

    char *xpdir = mkpathname(plugindir, "output", NULL);

    char *config_path = mkpathname(plugindir, "pnlren.conf", NULL);

    char *shader_dir = mkpathname(plugindir, "shaders", NULL);

    lacf_free(xpdir);

    lacf_free(config_path);
    
    XPLMRegisterFlightLoopCallback(NetworkingFlightLoopCallback, 1, NULL); // This will get called during each flight loop and we will handle network events

    DatarefPublisher::getInstance().Start();

    return (1);
}

PLUGIN_API void
XPluginDisable(void)
{
    //XPLMUnregisterDrawCallback(draw_cb, xplm_Phase_Window, 1, NULL);

    DatarefPublisher::getInstance().Finish();

    XPLMUnregisterFlightLoopCallback(NetworkingFlightLoopCallback, NULL);    //  Don't forget to unload this callback.  
}

#define XPILOT_NEW_MESSAGE 0x8534701

PLUGIN_API void XPluginReceiveMessage(
XPLMPluginID    inFromWho,
int             inMessage,
void *          inParam)
{
    
    switch (inMessage) {

        case XPLM_MSG_PLANE_CRASHED:
            /* This message is sent to your plugin whenever the user's plane crashes.      */

            break;

        case XPLM_MSG_PLANE_LOADED:
            /* This message is sent to your plugin whenever a new plane is loaded.  The    *
             * parameter is the number of the plane being loaded; 0 indicates the user's   *
             * plane.                                                                      */
            
            //NOTE: This is an absurd aspect of the XPLM that a void* is actually an int!
            if (inParam == 0) {
                logMsg("Aircraft loaded...");
                new_aircraft_loaded();
            }

        case XPILOT_NEW_MESSAGE:
            if (inParam) {
                std::string message(static_cast<char*>(inParam));
                logMsg("[ATC] Received new ATC message from xPilot:\n%s", message.c_str());
            }
        default:
            break;
    }
}
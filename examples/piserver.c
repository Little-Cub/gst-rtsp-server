#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

const char* IP = "141.219.243.219";
const char* PORT = "8554";

/* this timeout is periodically run to clean up the expired sessions from the
 * pool. This needs to be run explicitly currently but might be done
 * automatically as part of the mainloop. */
static gboolean timeout (GstRTSPServer * server)
{
  GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);
  gst_rtsp_session_pool_cleanup (pool);
  g_object_unref (pool);

  return TRUE;
}

int main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();

  gst_rtsp_server_set_address(server, IP);
  gst_rtsp_server_set_service(server, PORT);


  mounts = gst_rtsp_server_get_mount_points (server);

  
  factory = gst_rtsp_media_factory_new ();
  // gst_rtsp_media_factory_set_launch (factory, "( "
  //     "/dev/video0 ! video/x-raw,width=352,height=288,framerate=30/1 ! "
  //     "x264enc ! rtph264pay name=pay0 pt=96 "
  //     "audiotestsrc ! audio/x-raw,rate=8000 ! "
  //     "alawenc ! rtppcmapay name=pay1 pt=97 " ")");
  gst_rtsp_media_factory_set_launch(factory, "( "
      "v4l2src device=/dev/video0 ! video/x-raw,format=YUY2,width=352,height=288,framerate=30/1 ! "
      "videoconvert ! video/x-raw,format=I420 ! "
      "x264enc speed-preset=ultrafast ! rtph264pay name=pay0 pt=96 "
      ")");

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory (mounts, "/test", factory);

  /* don't need the ref to the mapper anymore */
  g_object_unref (mounts);

  /* attach the server to the default maincontext */
  if (gst_rtsp_server_attach (server, NULL) == 0)
    goto failed;

  /* add a timeout for the session cleanup */
  g_timeout_add_seconds (2, (GSourceFunc) timeout, server);

  /* start serving, this never stops */
#ifdef WITH_TLS
  g_print ("stream ready at rtsps://141.219.242.127:8554/test\n");
#else
  g_print ("stream ready at rtsp://%s:%s/test\n", IP, PORT);
#endif
  g_main_loop_run (loop);

  return 0;

  /* ERRORS */
failed:
  {
    g_print ("failed to attach the server\n");
    return -1;
  }
}

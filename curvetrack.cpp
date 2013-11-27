#include <string>

#include "cmdln.h"
#include "CurveFitProcessor.hpp"

using namespace cv;
using namespace std;




static void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
            "You select a color objects such as your face and it tracks it.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   camshiftdemo -c [camera_number]\n"
            "   camshiftdemo -f input_movie\n";

    cout << "\n\nHot keys: \n"
            "\tESC - quit the program\n"
            "\tc - stop the tracking\n"
            "\tb - switch to/from backprojection view\n"
            "\th - show/hide object histogram\n"
            "\tp - pause video\n"
            "To initialize tracking, select the object with mouse\n";
}


int main(int argc, char** argv)
{
    VideoCapture                cap;
    cmdln::parser_t             cmd_ln("Curve Fitting Object Tracker");
    cmdln::opt_val_t<int>       rotate("r", "rotate", "Rotate video images.", 0);
    cmdln::opt_val_t<string>    file("f", "file", "Use file for video source", "");
    cmdln::opt_val_t<int>       scale("s", "scale", "Scale input images", 1);
    cmdln::opt_val_t<int>       camNum("c", "camera", "input camera device", -1);
    cmdln::opt_val_t<bool>      paused("p", "pause", "Pause playback on start", true);
    cmdln::opt_val_t<int>       vmin("", "vmin", "input camera device", 10);
    cmdln::opt_val_t<int>       vmax("", "vmax", "input camera device", 256);
    cmdln::opt_val_t<int>       smin("", "smin", "input camera device", 30);
    cmdln::opt_val_t<int>       x("x", "xcoord", "Selection x-coordinate", 0);
    cmdln::opt_val_t<int>       y("y", "ycoord", "Selection y-coordinate", 0);
    cmdln::opt_val_t<int>       h("g", "height", "Selection input height", 0);
    cmdln::opt_val_t<int>       w("w", "width", "Selection input width", 0);



    cmd_ln.add(rotate);
    cmd_ln.add(file);
    cmd_ln.add(scale);
    cmd_ln.add(camNum);
    cmd_ln.add(paused);
    cmd_ln.add(vmin);
    cmd_ln.add(vmax);
    cmd_ln.add(smin);
    cmd_ln.add(x);
    cmd_ln.add(y);
    cmd_ln.add(h);
    cmd_ln.add(w);


    try
    {
        cmd_ln.parse(argc, argv);

        if ( camNum >= 0 ) {
            cout << "Using camera " << camNum.value() << endl;
            cap.open( camNum.value() );
        } else {
            cout << "Using file " << file.value() << endl;
            cap.open( file.value().c_str() );
        }

        help();

        if ( !cap.isOpened() ) {
            cout << "***Could not initialize capturing...***\n";
            cout << "Current parameter's value: \n";

            return -1;
        }

        CamShiftProcessor     *camshift = new CurveFitProcessor(cap, "Curve Fit");

        camshift->SetTransform(rotate, scale);
        camshift->SetThresholds(vmin, vmax, smin);

        // If an initial region selection was provided on command line, set
        // the selection in the video processor.
        if (w > 0 && h > 0)
        {
            camshift->SetSelection( Rect(x, y, w, h) );
        }

        camshift->Play(paused);
        delete camshift;
    }
    catch (cmdln::help_exception_t he)
    {
        cout << he.what() << endl;
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
    }
    


    return 0;
}

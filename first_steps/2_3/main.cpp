/// Example 2.3 : Video pipeline with ImageCroppingCalculator (dynamic crop)
/// By Oleksiy Grechnyev, IT-JIM
/// Here I show how to use ImageCroppingCalculator with a dynamic crop
/// Translation: the cropping rect is no longer supplied in calculator options,
/// But cropping rects for each frame come from a separate input stream in_rect
/// ACHTUNG! This pipeline is still NOT real-time!


#include <iostream>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <cmath>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

#include "mediapipe/framework/formats/rect.pb.h"

//==============================================================================
mediapipe::Status run() {
    using namespace std;
    using namespace mediapipe;
    
    // A graph with ImageCroppingCalculator 
    // Here we have a second inpt stream in_rect
    // Which contains the cropping rect for each frame, note the tag RECT
    string protoG = R"(
        input_stream: "in"
        input_stream: "in_rect"
        output_stream: "out"
        node {
            calculator: "ImageCroppingCalculator"
            input_stream: "IMAGE:in"
            input_stream: "RECT:in_rect"
            output_stream: "IMAGE:out"
        }
        )";

    // Parse config and create graph
    CalculatorGraphConfig config;
    if (!ParseTextProto<mediapipe::CalculatorGraphConfig>(protoG, &config)) {
        return absl::InternalError("Cannot parse the graph config !");
    } 
    CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));
    
    // Mutex protecting imshow() and the stop flag
    mutex mutexImshow;
    atomic_bool flagStop(false);

    // Add observer to "out", then start the graph
    // This callback displays the frame on the screen
    auto cb = [&mutexImshow, &flagStop](const Packet &packet)->Status{

        // Get cv::Mat from the packet
        const ImageFrame & outputFrame = packet.Get<ImageFrame>();
        cv::Mat ofMat = formats::MatView(&outputFrame);
        cv::Mat frameOut;
        cvtColor(ofMat, frameOut, cv::COLOR_RGB2BGR);
        cout << packet.Timestamp() << ": RECEIVED VIDEO PACKET size = " << frameOut.size() << endl;

        {
            lock_guard<mutex> lock(mutexImshow);
            // Display frame on screen and quit on ESC
            cv::imshow("frameOut", frameOut);
            if (27 == cv::waitKey(1)){
                cout << "It's time to QUIT !" << endl;
                flagStop = true;
            }
        }
        return OkStatus();
    };
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream("out", cb));
    graph.StartRun({});
    
    // Start the camera and check that it works
    cv::VideoCapture cap(cv::CAP_ANY);
    if (!cap.isOpened())
            return absl::NotFoundError("CANNOT OPEN CAMERA !");
    cv::Mat frameIn, frameInRGB;

    // Camera loop, runs until we get flagStop == true
    for (int i=0; !flagStop ; ++i){
        // Read next frame from camera
        cap.read(frameIn);
        if (frameIn.empty())
            return absl::NotFoundError("CANNOT OPEN CAMERA !");

        cout << "SIZE_IN = " << frameIn.size() << endl;
        {
            lock_guard<mutex> lock(mutexImshow);
            cv::imshow("frameIn", frameIn);
        }

        // Convert it to a packet and send
        cv::cvtColor(frameIn, frameInRGB, cv::COLOR_BGR2RGB);
        ImageFrame *inputFrame =  new ImageFrame(
            ImageFormat::SRGB, frameInRGB.cols, frameInRGB.rows, ImageFrame::kDefaultAlignmentBoundary
        );
        frameInRGB.copyTo(formats::MatView(inputFrame));
        Timestamp ts(i);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", 
            Adopt(inputFrame).At(ts)
        ));

        // Create a crop rect (center+width+height) for each frame
        // Let's move the rect up and down for fun
        int yc = int(frameIn.rows * (0.5 + 0.2 * cos(0.3 * i)));
        Rect rect;
        rect.set_width(0.8*frameIn.cols);
        rect.set_height(0.4*frameIn.rows);
        rect.set_x_center(frameIn.cols / 2);
        rect.set_y_center(yc);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in_rect", MakePacket<Rect>(rect).At(ts)));
    }
    // Don't forget to close both input streams !
    graph.CloseInputStream("in");
    graph.CloseInputStream("in_rect");
    // Wait for the graph to finish
    MP_RETURN_IF_ERROR(graph.WaitUntilDone());
    return OkStatus();
}

//==============================================================================
int main(int argc, char** argv){
    using namespace std;

    FLAGS_alsologtostderr = 1;
    google::SetLogDestination(google::GLOG_INFO, ".");
    google::InitGoogleLogging(argv[0]);
    
    cout << "Example 2.3 : Video pipeline with ImageCroppingCalculator (dynamic crop)" << endl;
    mediapipe::Status status = run();
    cout << "status =" << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}

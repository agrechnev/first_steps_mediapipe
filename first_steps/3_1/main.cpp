/// Example 3.1 : Why MediaPipe is not real-time?
/// By Oleksiy Grechnyev, IT-JIM
/// Here we use SlowCalculator which simulates some slow image-processing operation
/// By default, MP is not real-time. It queues all received package until
/// they are eventually processed by SlowCalculator.
/// This is the idea at least. No packages are ever lost.
/// Since input packets arrive in real-time, and their number is unlimited,
/// it causes an ever-increased lag until the program fills all RAM and crashes.

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

//==============================================================================
mediapipe::Status run() {
    using namespace std;
    using namespace mediapipe;
    
    // A graph with SlowCalculator and nothing else
    string protoG = R"(
        input_stream: "in"
        output_stream: "out"
        node {
            calculator: "SlowCalculator"
            input_stream: "IMAGE:in"
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

    }
    // Close the input streams, Wait for the graph to finish
    graph.CloseInputStream("in");
    MP_RETURN_IF_ERROR(graph.WaitUntilDone());
    return OkStatus();
}

//==============================================================================
int main(int argc, char** argv){
    using namespace std;

    FLAGS_alsologtostderr = 1;
    google::SetLogDestination(google::GLOG_INFO, ".");
    google::InitGoogleLogging(argv[0]);
    
    cout << "Example 3.1 : Why MediaPipe is not real-time? " << endl;
    mediapipe::Status status = run();
    cout << "status =" << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}

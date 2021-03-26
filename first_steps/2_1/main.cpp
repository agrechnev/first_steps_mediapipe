/// Example 2.1 : Video pipeline
/// By Oleksiy Grechnyev, IT-JIM
/// Here I create our very first video pipeline

/// Here we use OpenCV, and MP has its own wrappers for opencv headers (How stupid is this?) 
/// MP uses an mutable class ImageFrame for images
/// One must convert cv::Mat <-> ImageFrame back and forth
/// Note that cv::Mat uses BGR by default, while ImageFrame uses RGB by default
/// We stick to this convention and put cv::cvtColor when needed

/// ACHTUNG! This pipeline is NOT real-time!
/// With the default execution policy, MP never loses packets
/// While it's unlikely to happen with PassThroughCalculator,
/// If our pipeline was too slow, the packets would 
/// queue indefinitely, increasing the lag, and eventually 
/// crashing the computer.
/// Hopefully I'll make a real-time example later.


#include <iostream>
#include <string>
#include <memory>
#include <atomic>

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
    
    // A rather trivial graph by now
    string protoG = R"(
        input_stream: "in",
        output_stream: "out",
        node {
            calculator: "PassThroughCalculator",
            input_stream: "in",
            output_stream: "out",
        }
        )";
    // Parse config and create graph
    CalculatorGraphConfig config;
    if (!ParseTextProto<mediapipe::CalculatorGraphConfig>(protoG, &config)) {
        // mediapipe::Status is actually absl::Status (at least in the current mediapipe)
        // So we can create BAD statuses like this
        return absl::InternalError("Cannot parse the graph config !");
    } 
    CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));
    
    // Add observer to "out", then start the graph
    // This callback displays the frame on the screen
    auto cb = [](const Packet &packet)->Status{
        cout << packet.Timestamp() << ": RECEIVED VIDEO PACKET !" << endl;

        // Get data from packet (you should be used to this by now)
        const ImageFrame & outputFrame = packet.Get<ImageFrame>();
        // Represent ImageFrame data as cv::Mat (MatView is a thin wrapper, no copying)
        cv::Mat ofMat = formats::MatView(&outputFrame);
        // Convert RGB->BGR
        cv::Mat frameOut;
        cvtColor(ofMat, frameOut, cv::COLOR_RGB2BGR);
        // Display frame on screen and quit on ESC
        // Returning non-OK status aborts graph execution
        // I'll make a nicer quit in later examples
        cv::imshow("frameOut", frameOut);
        if (27 == cv::waitKey(1))
            // I was not sure which Abseil error to use here ...
            return absl::CancelledError("It's time to QUIT !");
        else
            return OkStatus();
    };
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream("out", cb));
    graph.StartRun({});
    
    // Start the camera and check that it works
    cv::VideoCapture cap(cv::CAP_ANY);
    if (!cap.isOpened())
            return absl::NotFoundError("CANNOT OPEN CAMERA !");
    cv::Mat frameIn, frameInRGB;

    // Endless loop over frames
    for (int i=0; ; ++i){
        // Read next frame from camera
        cap.read(frameIn);
        if (frameIn.empty())
            return absl::NotFoundError("CANNOT OPEN CAMERA !");
        // Convert BGR to RGB
        cv::cvtColor(frameIn, frameInRGB, cv::COLOR_BGR2RGB);
        // Create an empty (black?) RGB ImageFrame with the same size as our image
        // Note the raw pointer. Is there a memory leak?
        // NO, because of Adopt(), see below
        ImageFrame *inputFrame =  new ImageFrame(
            ImageFormat::SRGB, frameInRGB.cols, frameInRGB.rows, ImageFrame::kDefaultAlignmentBoundary
        );
        // Copy data from cv::Mat to Imageframe, using
        // MatView: a cv::Mat representation of ImageFrame
        frameInRGB.copyTo(formats::MatView(inputFrame));
        
        // Create and send a video packet
        uint64 ts = i;
        // Adopt() creates a new packet from a raw pointer, and takes this pointer under MP management
        // So that you must not call delete on the pointer after that
        // MP will delete your object automatically when the packet is destroyed
        // This is like creating shared_ptr from a raw pointer
        // This is useful for the classes which cannot be (easily) copied, like ImageFrame
        // Note that MakePacket<...>() we used previously contains a move or copy operation 
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", 
            Adopt(inputFrame).At(Timestamp(ts))
        ));
    }
    // We never reach here, the status is always error on exit
    // MP_RETURN_IF_ERROR(graph.WaitUntilDone());
    // return OkStatus();
}

//==============================================================================
int main(int argc, char** argv){
    using namespace std;
    
    cout << "Example 2.1 : Video pipeline" << endl;
    mediapipe::Status status = run();
    cout << "status =" << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}

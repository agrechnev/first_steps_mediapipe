/// Example 2.2 : Video pipeline with ImageCroppingCalculator and ScaleImageCalculator
/// By Oleksiy Grechnyev, IT-JIM
/// Here I show how to use standard image claculators from MP
/// ACHTUNG! This pipeline is still NOT real-time!


#include <iostream>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>

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
    
    // A graph with ImageCroppingCalculator and ScaleImageCalculator

    // First, we use ImageCroppingCalculator
    // It crops (cuts) a rectangle from the image
    // The desired rectangle can be specified either staticaly (via options)
    // Or dynamically (via another stream)
    // Here we use the static way
    // Note that standard MP calculators often use tags, here it's "IMAGE"

    // Second, it is ScaleImageCalculator to scale image to 640x480 (also set statically via the options)
    // Note: No tags this time. Doesn't work with tags. Why, google, why?
    // Making graphs is fun, isn't it?
    string protoG = R"(
        input_stream: "in"
        output_stream: "out"
        node {
            calculator: "ImageCroppingCalculator"
            input_stream: "IMAGE:in"
            output_stream: "IMAGE:out1"
            options: {
                [mediapipe.ImageCroppingCalculatorOptions.ext] {
                    norm_width: 0.8
                    norm_height: 0.4
                }
            }
        }
        node {
            calculator: "ScaleImageCalculator"
            input_stream: "out1"
            output_stream: "out"
            options: {
                [mediapipe.ScaleImageCalculatorOptions.ext] {
                    target_width: 640
                    target_height: 480
                    preserve_aspect_ratio: false
                    algorithm: CUBIC
                }
            }
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
    
    // Now we use cv::imshow() in 2 different threads
    // For me it worked even without mutex, but let's protect imshow()
    // wth a mutex to be safe!
    mutex mutexImshow;

    // We use an atomic stop flag for a soft quit, without giving graph errors
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

        // Let's protect cv::imshow() by the mutex
        {
            lock_guard<mutex> lock(mutexImshow);
            // Display frame on screen and quit on ESC
            cv::imshow("frameOut", frameOut);
            if (27 == cv::waitKey(1)){
                cout << "It's time to QUIT !" << endl;
                // Set the stop flag, to finish the camera loop
                flagStop = true;
            }
        }
        // ALways return OK now: No errors!
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

        // Show input frame, in a different thread from the callback, mutex-protected
        // Note: no waitKey() here, we only put waitKey() in the callback
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
        uint64 ts = i;
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", 
            Adopt(inputFrame).At(Timestamp(ts))
        ));
    }
    // Now we can reach here!
    // Don't forget to close the input stream !
    graph.CloseInputStream("in");
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
    
    cout << "Example 2.2 : Video pipeline with ImageCroppingCalculator" << endl;
    mediapipe::Status status = run();
    cout << "status =" << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}

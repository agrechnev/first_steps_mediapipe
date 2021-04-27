#include <chrono>
#include <thread>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

//==============================================================================
namespace mediapipe {
    /// A custom image-processing calculator
    /// It applies photo-negative to the central 1/9 of the image
    /// The catch: we slow it down deliberately with a 0.2s delay (5 ~fps)
    /// To simulate the effect of a slow image-processing calcualtor
    class SlowCalculator : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            using namespace std;
            // 1 input: image and keypoints
            cc->Inputs().Tag("IMAGE").Set<ImageFrame>();
            // 1 output: image with keypoints painted
            cc->Outputs().Tag("IMAGE").Set<ImageFrame>();
            return OkStatus();
        }
        
        Status Process(CalculatorContext *cc) override {
            using namespace std;
            using namespace cv;
            // Get input packets 
            Packet pIn = cc->Inputs().Tag("IMAGE").Value();

            // Create a new ImageFrame by copying the one from from pIn, then modify the copy
            ImageFrame *iFrame = new ImageFrame();
            iFrame->CopyFrom(pIn.Get<ImageFrame>(), 1);
            Mat img = formats::MatView(iFrame);

            // Apply photo negative to img central 1/9
            int nc = img.cols / 3, nr = img.rows / 3;
            Rect r(nc, nr, nc, nr);
            Mat m(img, r);
            bitwise_not(m, m);

            // Slow down artificially: wait for 200 ms !
            this_thread::sleep_for(chrono::milliseconds(200));
            
            // Create output packet from iFrame and send it
            Packet pOut = Adopt<ImageFrame>(iFrame).At(cc->InputTimestamp());
            cc->Outputs().Tag("IMAGE").AddPacket(pOut);
            return OkStatus();
        }
    };
    REGISTER_CALCULATOR(SlowCalculator);
}
//==============================================================================
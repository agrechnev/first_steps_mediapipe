#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

//==============================================================================
namespace mediapipe {
    /// A custom image-processing calculator
    /// It draws keypoints on an image
    class DrawFeatCalculator24 : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            using namespace std;
            // 2 input: image and keypoints
            cc->Inputs().Tag("IMAGE").Set<ImageFrame>();
            cc->Inputs().Tag("FEATURES").Set<std::vector<cv::KeyPoint>>();
            // 1 output: image with keypoints painted
            cc->Outputs().Tag("IMAGE").Set<ImageFrame>();
            return OkStatus();
        }
        
        Status Process(CalculatorContext *cc) override {
            using namespace std;
            // Get input packets 
            Packet pIn = cc->Inputs().Tag("IMAGE").Value();
            Packet pFe = cc->Inputs().Tag("FEATURES").Value();
            const vector<cv::KeyPoint> &kps = pFe.Get<vector<cv::KeyPoint>>();
            // Note: as package are immutable, it is not allowed to paint on the input image !!!
            // Here we create a new ImageFrame by copying the one from from pIn, then paint on the copy
            ImageFrame *iFrame = new ImageFrame();
            iFrame->CopyFrom(pIn.Get<ImageFrame>(), 1);
            cv::Mat img = formats::MatView(iFrame);
            for (const cv::KeyPoint &kp: kps) {
                cv::circle(img, kp.pt, 3, cv::Scalar(0xff, 0, 0), 1);
            }
            
            // Create output packet from iFrame and send it
            Packet pOut = Adopt<ImageFrame>(iFrame).At(cc->InputTimestamp());
            cc->Outputs().Tag("IMAGE").AddPacket(pOut);
            return OkStatus();
        }
    };
    REGISTER_CALCULATOR(DrawFeatCalculator24);
}
//==============================================================================
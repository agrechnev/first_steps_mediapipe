#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/examples/first_steps/1_4/goblin_calculator14.pb.h"

//==============================================================================
namespace mediapipe{
    /// A calculator with options
    /// Applies the operation f(x) = a*x + b to a stream of real numbers
    class GoblinCalculator14 : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            using namespace std;
            // Specify a calculator with 1 input, 1 output, both of type double
            cc->Inputs().Index(0).Set<double>();
            cc->Outputs().Index(0).Set<double>();
            return OkStatus(); 
        }

        /// Open method is executed once when starting the graph
        /// It can be used to parse options
        Status Open(CalculatorContext *cc) override {
            using namespace std;
            // Copy options to class fields (a good style, but in principle we can access them directly in Process)
            auto options = cc->Options<GoblinCalculator14Options>();
            a = options.opt_a();
            b = options.opt_b();
            cout << "GoblinCalculator14::Open() : a = " << a << ", b = " << b << endl;
            return OkStatus(); 
        }

        Status Process(CalculatorContext *cc) override {
            using namespace std;
            // Receive the input packet, extract double
            Packet pIn = cc->Inputs().Index(0).Value();
            double x = pIn.Get<double>();
            // Process the number
            double y = x * a + b;
            // Create the output packet, then send
            Packet pOut = MakePacket<double>(y).At(cc->InputTimestamp());
            cc->Outputs().Index(0).AddPacket(pOut);
            return OkStatus(); 
        }
    private:
        // Keep our options here
        double a, b;
    };

     REGISTER_CALCULATOR(GoblinCalculator14);
}

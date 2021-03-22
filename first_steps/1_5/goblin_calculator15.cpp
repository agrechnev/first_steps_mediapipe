#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"

//==============================================================================
namespace mediapipe{
    /// A calculator with input and output side packets
    class GoblinCalculator15 : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            using namespace std;
            // Specify a calculator with 1 input, 1 output, both of type double
            cc->Inputs().Index(0).Set<double>();
            cc->Outputs().Index(0).Set<double>();
            // Specify side packets
            cc->InputSidePackets().Index(0).Set<double>();
            return OkStatus(); 
        }

        /// Open method is executed once when starting the graph
        /// It can be used to parse input side packets, they are already available
        Status Open(CalculatorContext *cc) override {
            using namespace std;
            // Extract a,b from input side packets
            Packet p = cc->InputSidePackets().Index(0);
            a = p.Get<double>();
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
        double a=2, b=0;
    };

     REGISTER_CALCULATOR(GoblinCalculator15);
}

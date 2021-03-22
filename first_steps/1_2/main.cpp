/// Example 1.2 : Our first custom calculator
/// By Oleksiy Grechnyev, IT-JIM
/// Here we create our first custom calculator
/// A Calculator in MP is a type used to create graph nodes
/// The same calculator can  be used for multiple nodes

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"


//==============================================================================
// All calculators must be in namespace mediapipe (ugly, I know)
namespace mediapipe{
    /// Our first calculator : This calculator processes a double number
    class GoblinCalculator12 : public CalculatorBase {
    public:
        /// Every calculator must implement the static GetContract() method
        /// Which defines allowed inputs and outputs
        static Status GetContract(CalculatorContract *cc) {
            using namespace std;
            // Specify a calculator with 1 input, 1 output, both of type double
            cc->Inputs().Index(0).Set<double>();
            cc->Outputs().Index(0).Set<double>();
            return OkStatus(); // Never forget to say "OK" !
        }

        /// The method Process() receives input packets and issues output packets
        /// It is called for every timestamp for which input packets are available
        /// Synchronization: MP automatically synchronizes streams
        /// If there are several input streams, all input packets have the same timestamp
        /// All operations are performed through the CalculatorContext object
        Status Process(CalculatorContext *cc) override {
            using namespace std;
            // Receive the input packet
            Packet pIn = cc->Inputs().Index(0).Value();
            // Extract the double number
            // A Packet is immutable, we cannot edit it!
            double x = pIn.Get<double>();
            // Process the number
            double y = x * 2;
            // Create the output packet, with the input timestamp
            Packet pOut = MakePacket<double>(y).At(cc->InputTimestamp());
            // Send it to the output stream
            cc->Outputs().Index(0).AddPacket(pOut);
            return OkStatus(); // Never forget to say "OK" !
        }
     };

     // We must register our calculator with MP, so that it can be used in graphs
     REGISTER_CALCULATOR(GoblinCalculator12);
}

//==============================================================================
mediapipe::Status run(){
    using namespace std;
    // Create the graph using our custom calculator
    string protoG = R"(
    input_stream: "in"
    output_stream: "out"
    node {
        calculator: "GoblinCalculator12"
        input_stream: "in"
        output_stream: "out"
    }
    )";

    // Let us now parse the string properly this time, checking if the parse is successful
    mediapipe::CalculatorGraphConfig config;
    if (!mediapipe::ParseTextProto<mediapipe::CalculatorGraphConfig>(protoG, &config)) {
        // mediapipe::Status is actually absl::Status (at least in the current mediapipe)
        // So we can create BAD statuses like this
        return absl::InternalError("Cannot parse the graph config !");
    } 

    // Create MP Graph and intialize it with config
    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    // Add observer to "out:
    auto cb = [](const mediapipe::Packet &packet)->mediapipe::Status{
        cout << packet.Timestamp() << ": RECEIVED PACKET " << packet.Get<double>() << endl;
        return mediapipe::OkStatus();
    };
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream("out", cb));

    /// Run the graph
    MP_RETURN_IF_ERROR(graph.StartRun({}));

    // Send input packets to the graph, stream "in", then close it
    for (int i=0; i<13; ++i) {
        mediapipe::Timestamp ts(i);
        mediapipe::Packet packet = mediapipe::MakePacket<double>(i*0.1).At(ts);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
    }
    graph.CloseInputStream("in");

    // Wait for the graph to finish
    MP_RETURN_IF_ERROR(graph.WaitUntilDone());
    return mediapipe::OkStatus();
}

//==============================================================================
int main(){
    using namespace std;
    cout << "Example 1.2 : Our first custom calculator ..." << endl;
    // Call run(), which return a status
    mediapipe::Status status = run();
    cout << "status = " << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}
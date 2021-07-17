#include <opcua/Connection.h>
#include <iostream>

using namespace OWA::OpcUa;
int main (int argc, char** argv)
{
  (void)argc;
  (void)argv;
  const std::string value1 = "Hello World!";
  const std::string value2 = "Hi there!";
  bool succeeded = false;

  // initSdk must be called in order to initialize logging subsystem:
  OWA::OpcUa::Utils::initSdk();

  {
    auto connection = Connection::create("opc.tcp://opcuaserver.com:48010", true);
    if (connection->connect().get().isGood())
    {
      // Read from a node and write to it.
      {
        NodeId nodeId("Demo.Static.Scalar.String", 2);
        ReadRequest::Ptr readRequest(new ReadRequest(nodeId));
        auto readResponse = connection->send(readRequest).get();
        if (readResponse->isGood() && readResponse->results.size() == 1 && Utils::isGood(readResponse->results[0].statusCode))
        {
          // We know that data type of the value is String, therefore convert it to string should succeed:
          std::string currentValue = readResponse->results[0].value;
          std::string newValue = (currentValue == value1) ? value2 : value1;
          WriteRequest::Ptr writeRequest(new WriteRequest(WriteValue(nodeId, DataValue(Variant(newValue)))));
          auto writeResponse = connection->send(writeRequest).get();
          if (writeResponse->isGood() && Utils::isGood(writeResponse->results[0]))
          {
            readResponse = connection->send(readRequest).get();
            if (readResponse->isGood() && readResponse->results.size() == 1 && Utils::isGood(readResponse->results[0].statusCode))
            {
              currentValue = readResponse->results[0].value.toString();
              if (currentValue == newValue)
              {
                std::cout << "Wrote value " << currentValue << " to the server!" << std::endl;
                succeeded = true;
              }
            }
          }
        }
      }

      {
        ReadRequest::Ptr readRequest(new ReadRequest(2258));

        DateTime startTime(true);
        do {
          auto response =  connection->send(readRequest).get();
          if (response->isGood())
          {
            if (response->results.size() == 1 && Utils::isGood(response->results[0].statusCode))
              std::cout << "Read value = " << response->results[0].value.toString() << std::endl;
            else
            {
              // TODO
            }
          }
          else
          {

            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } while (DateTime::diffInMinutes(startTime, DateTime(true)) < 65);

        std::cout << "Read for " << DateTime::diffInSeconds(startTime, DateTime(true)) << " seconds." << std::endl;
      }
    }
  }
  OWA::OpcUa::Utils::closeSdk();
  if (!succeeded)
  {
    std::cout << "Writing new value to the server failed!" << std::endl;
    return -1;
  }
  else
  {
    return 0;
  }
}

#include <opcua/Connection.h>
#include <iostream>
#include <iomanip>

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
    std::string endpointUrl;

    endpointUrl = "opc.tcp://opcuaserver.com:48010";
    // endpointUrl = "opc.tcp://localhost:48010";
    
    // The Connection instance can be created in simpler way too, if support for Complex types is not required, and default configuration is OK:
    // auto connection = Connection::create(endpointUrl, true);

    ClientConfiguration config(endpointUrl);
    config.readTypeDefinitionsOnConnect = true;
    config.readXmlTypeDictionaryOnConnect = true;
    config.createSession = true;

    config.securityMode = SecurityMode(SecurityPolicyId::None, MessageSecurityMode::None);

    // To connect in secured mode, uncomment line below.
    config.securityMode = SecurityMode(SecurityPolicyId::Basic256Sha256, MessageSecurityMode::SignAndEncrypt);
    
    // If the option acceptAnyCertificate is false, then connections only to servers with trusted certificate will be allowed.
    // If certificate is not valid or trusted, it will be saved in the data/PKI/rejected folder. 
    // To trust the certificate, move it to the folder data\PKI\trusted\certs
    config.certificateSettings.validationRules.acceptAnyCertificate = false;
    
    // Default identity token is Anonymous.
    // Setting username/password token below:
    auto userNameIdentityToken = std::make_shared<UserNameIdentityToken>();
    // Default credentials in Unified automation Demo Server.
    userNameIdentityToken->userName = "root";   
    userNameIdentityToken->password = "secret";

    // To connect using Anonymous token, uncomment line below:
    config.identityToken = userNameIdentityToken;

    auto connection = Connection::create(config);

    // Namespace manager holds information about server namespace array and complex data types.
    // Required if you need to expand complex type values.
    std::shared_ptr < NamespaceManager> namespaceManager;

    connection->SetNamespaceManagerUpdatedCallback([&namespaceManager](std::shared_ptr < NamespaceManager> nsm) {
      namespaceManager = nsm;

      // Namespace settings can be serialized to XML string and saved:
      std::string nsmContent;
      if (nsm->serializeToString(nsmContent).isGood())
      {
        auto saveResult = Utils::saveStringToFile(nsmContent, "./namespaceSettings.xml");
      }
    });

    auto connectResult = connection->connect().get();
    if (!connectResult.isGood())
    {
      std::cerr << "Failed to connect, error: " << connectResult.toString() << std::endl;
    }
    else
    {
      // Example how to read a few variables:
      {
        NodeId nodeId(4294967295, 2);
        
        // Alternatively, node id can be initialized from serialized string:
        NodeId nid;
        if (!nid.parse("ns=2;i=4294967295"))
        {
          std::cout << "Failed to parse node id" << std::endl;
        }

        ReadRequest::Ptr readRequest(new ReadRequest(nodeId));
        readRequest->nodesToRead.push_back(nid);
        readRequest->nodesToRead.push_back(NodeId("abra-cadabra")); // Node with this ID does not exist in the address space

        auto readResponse = connection->send(readRequest).get();
        if (readResponse->isGood() && readResponse->results.size() == readRequest->nodesToRead.size())
        {
          auto requestIter = readRequest->nodesToRead.begin();
          for (auto result = readResponse->results.begin(); result != readResponse->results.end(); result++, requestIter++)
          {
            if (Utils::isGood(result->statusCode))
            {
              std::cout << "Node " << requestIter->nodeId.toString() << " has value " << result->value.toString() << std::endl;
            }
            else
            {
              std::cout << "Failed to read from node " << requestIter->nodeId.toString() << ", error: " << Utils::toString(result->statusCode) << std::endl;
            }
          }
        }
      }
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
          
          // Example of write request writing to the variable of 32 bit unsigned integer type.
          // Here constructor of the Variant takes value of explicitly defined data type uint32_t:
          {
            WriteRequest::Ptr writeRequest2(new WriteRequest(WriteValue(NodeId("Demo.Static.Scalar.UInt32", 2), DataValue(Variant((uint32_t)456)))));
          }

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

      // Write to the variable of known integer data type:
      {
        WriteRequest::Ptr writeRequest(new WriteRequest());

        NodeId nodeId("Demo.Static.Scalar.Byte", 2);
        DataValue dv;

        // Literal numeric value like 123 can be treated as of many data types: could be float, double or 8, 16, 32, 64 bit signed or unsigned integer.
        // OPC UA requires written values to have exactly the same data type as data type of the variable on the server. 
        // Therefore, explicitly convert it the data type of the variable, which in case of this variable is Byte.
        // C++ has native equivalent to it, so uint8_t is used to represent OPC UA Byte data type. 
        // As a result of using conversion to uint8_t, Variant variable will have data type set to Byte:
        dv.value = (uint8_t) 123;
        
        // Alternatively, we could assign to the Variant variable values of other variables, without explicit conversion, in this case variable's data type is already known to the compiler:
        uint8_t byteValue = 123;
        dv.value = byteValue;

        WriteValue wv;
        wv.nodeId = nodeId;
        wv.value = dv;
        writeRequest->nodesToWrite.push_back(wv);

        auto writeResponse = connection->send(writeRequest).get();
        if (writeResponse->isGood() && Utils::isGood(writeResponse->results[0]))
        {
          std::cout << "Wrote value " << dv.value.toString() << " to the variable with node id " << nodeId.toString() << std::endl;
        }
        else
        {
          std::cout << "Wrote value " << dv.value.toString() << " to the variable with node id " << nodeId.toString() << std::endl;
        }
      }

      // Create subscription and monitored items:
      {

        DateTime startTime(true);
        auto createSubscriptionRequest = std::make_shared<CreateSubscriptionRequest>();

        createSubscriptionRequest->requestedPublishingInterval = 1000;
        double samplingRate = 1000;
        uint32_t queueSize = 10;

        std::atomic<int> counter = 0;

        // Notification Observer type callback is called when Publish response is received from the server.
        NotificationObserver notificationCallback = [&counter, &namespaceManager](NotificationMessage& notificationMessage)
        {
          counter++;
          std::cout << "Got notification number " << counter << " with sequence number " << notificationMessage.sequenceNumber << std::endl;

          for (auto iter = notificationMessage.notificationData.begin(); iter != notificationMessage.notificationData.end(); iter++)
          {
            auto p = iter->get();
            DataChangeNotification* dcn = dynamic_cast<DataChangeNotification*>(p);
            if (dcn)
            {
              for (auto m = dcn->monitoredItems.begin(); m != dcn->monitoredItems.end(); m++)
              {
                std::cout << "Handle = " << m->clientHandle << ", timestamp = " << m->dataValue.sourceTimestamp.toString(true)
                  << ", value = " << m->dataValue.value.toJson(namespaceManager) << std::endl;
              }
            }
          }
        };
        auto createSubscriptionResponse = connection->send(createSubscriptionRequest, notificationCallback, false,
          [samplingRate, queueSize, connection]
        (std::shared_ptr<CreateSubscriptionRequest>& request, std::shared_ptr<CreateSubscriptionResponse>& response)
        {
            (void)request;
          if (response->isGood())
          {
            CreateMonitoredItemsRequest::Ptr createMonItemsRequest(new CreateMonitoredItemsRequest());

            createMonItemsRequest->subscriptionId = response->subscriptionId;

            createMonItemsRequest->itemsToCreate.push_back(MonitoredItemCreateRequest(NodeId(std::string("Demo.Dynamic.Scalar.String"), 2),
              samplingRate, queueSize));
            createMonItemsRequest->itemsToCreate.push_back(MonitoredItemCreateRequest(NodeId(std::string("Demo.Dynamic.Scalar.XmlElement"), 2),
              samplingRate, queueSize));

            // Server Status Node, complex type
            createMonItemsRequest->itemsToCreate.push_back(MonitoredItemCreateRequest(NodeId(2256), samplingRate, queueSize));

            for (int index = 0; index < 10; index++)
            {
              std::stringstream s;
              s << "Demo.Massfolder_Dynamic.Variable" << std::dec << std::setw(4) << std::setfill('0') << index;
              createMonItemsRequest->itemsToCreate.push_back(MonitoredItemCreateRequest(NodeId(s.str(), 2), samplingRate, queueSize));
            }

            auto response = connection->send(createMonItemsRequest, false).get();
            if (!response->isGood())
            {
              std::cerr << "Failed to send CreateMonitoredItems request" << std::endl;
            }
          }
          else
          {
            std::cerr << "Failed to create subscription, error: " << Utils::toString(response->header.serviceResult) << std::endl;
          }
          return true;
        });

        // Wait until 10 data change notification is received, or timeout:
        while (counter < 10 && DateTime::diffInMilliseconds(startTime, DateTime(true)).count() < 20000)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
      // Recursively browse OPC UA Server address space, using browse and Browse Next service calls.
      {
        DateTime startTime(true);
        auto browseRequest = std::make_shared<BrowseRequest>();
        const int maxLevel = 3;
        browseRequest->requestedMaxReferencesPerNode = 10;

        BrowseDescription bd;
        bd.browseDirection = BrowseDirection::forward;
        bd.includeSubtypes = true;
        bd.referenceTypeId = Ids::HierarchicalReferences;
        bd.nodeClassMask.value = 0;
        bd.nodeClassMask.bits.Object = 1;
        bd.nodeClassMask.bits.Variable = 1;
        bd.nodeId = 85; // Start browsing from Objects Node
        browseRequest->nodesToBrowse.push_back(bd);

        // Variables to call callback functions recursively. Passed to lambda functions by reference.
        std::function<bool(std::shared_ptr<BrowseRequest>& request, std::shared_ptr<BrowseResponse>& response)> cb;
        std::function<bool(std::shared_ptr<BrowseNextRequest>& request, std::shared_ptr<BrowseNextResponse>& response)> cbbn;

        // Variables to keep track of sent requests and received responses:
        std::atomic<int> numberOfRequests = 0;
        std::atomic<int> numberOfResponses = 0;
        std::atomic <int> totalNodes = 0;

        // This function is called when BrowseNext response is received.
        auto browseNextCallbackFunction = [connection, &cbbn, &cb, &numberOfRequests, &numberOfResponses, maxLevel, &totalNodes]
        (std::shared_ptr<BrowseNextRequest>& request, std::shared_ptr<BrowseNextResponse>& response)
        {
          std::pair<int, std::vector<NodeId>> requestContext = std::any_cast<std::pair<int, std::vector<NodeId>>>(request->context);
          int level = requestContext.first;

          if (!Utils::isGood(response))
          {
            std::cerr << "Browse request failed with error " << Utils::toString(response->header.serviceResult) << std::endl;
          }
          else if (response->results.size() != requestContext.second.size())
          {
            std::cerr << "Browse request has incorrect number of results " << response->results.size() << " vs. expected " << requestContext.second.size() << std::endl;
          }
          else
          {
            // Normal flow of the operation.

            auto browseNextRequest = std::make_shared<BrowseNextRequest>();

            auto nextLevelRequest = std::make_shared<BrowseRequest>();
            nextLevelRequest->context = requestContext.first + 1;

            auto reqIter = requestContext.second.begin();
            for (auto iter = response->results.begin(); iter != response->results.end(); iter++, reqIter++)
            {
              for (int index = 0; index < level; index++)
                std::cout << "\t";

              std::cout << "Browsed Node id = " << reqIter->toString() << std::endl;

              for (auto r = iter->references.begin(); r != iter->references.end(); r++)
              {
                totalNodes++;
                for (int index = 0; index < (level + 1); index++)
                  std::cout << "\t";
                std::cout << "Display name = " << r->displayName.toString() << ", node id = " << r->nodeId.toString() << std::endl;

                if (level < maxLevel)
                {
                  BrowseDescription bd;
                  bd.browseDirection = BrowseDirection::forward;
                  bd.includeSubtypes = true;
                  bd.referenceTypeId = Ids::HierarchicalReferences;
                  bd.nodeClassMask.value = 0;
                  bd.nodeClassMask.bits.Object = 1;
                  bd.nodeClassMask.bits.Variable = 1;
                  bd.nodeId = r->nodeId.nodeId;
                  nextLevelRequest->nodesToBrowse.push_back(bd);
                }
              }

              if (!iter->continuationPoint.empty())
              {
                browseNextRequest->continuationPoints.push_back(iter->continuationPoint);
              }
            }

            if (!browseNextRequest->continuationPoints.empty())
            {
              numberOfRequests++;
              connection->send(browseNextRequest, cbbn);
            }

            if (!nextLevelRequest->nodesToBrowse.empty())
            {
              numberOfRequests++;
              connection->send(nextLevelRequest, cb);
            }
          }
          numberOfResponses++;
          return true;
        };

        // This callback function is called when Browse response is received.
        auto callbackFunction = [maxLevel, connection, browseNextCallbackFunction, &cb, &numberOfRequests, &numberOfResponses, &totalNodes]
        (std::shared_ptr<BrowseRequest>& request, std::shared_ptr<BrowseResponse>& response)
        {
          int level = std::any_cast<int>(request->context);
          if (!Utils::isGood(response))
          {
            std::cerr << "Browse request failed with error " << Utils::toString(response->header.serviceResult) << std::endl;
          }
          else if (response->results.size() != request->nodesToBrowse.size())
          {
            std::cerr << "Browse response has incorrect number of results " << response->results.size() << " vs. expected " << request->nodesToBrowse.size() << std::endl;
          }
          else
          {
            // Normal flow of the operation.

            auto browseNextRequest = std::make_shared<BrowseNextRequest>();

            auto nextLevelRequest = std::make_shared<BrowseRequest>();
            nextLevelRequest->context = level + 1;

            auto reqIter = request->nodesToBrowse.begin();
            std::vector<NodeId> browseNextNodeIds;

            for (auto iter = response->results.begin(); iter != response->results.end(); iter++, reqIter++)
            {
              for (int index = 0; index < level; index++)
                std::cout << "\t";

              std::cout << "Browsed Node id = " << reqIter->nodeId.toString() << std::endl;

              for (auto r = iter->references.begin(); r != iter->references.end(); r++)
              {
                totalNodes++;
                for (int index = 0; index < (level + 1); index++)
                  std::cout << "\t";
                std::cout << "Display name = " << r->displayName.toString() << ", node id = " << r->nodeId.toString() << std::endl;

                if (level < maxLevel)
                {
                  BrowseDescription bd;
                  bd.browseDirection = BrowseDirection::forward;
                  bd.includeSubtypes = true;
                  bd.referenceTypeId = Ids::HierarchicalReferences;
                  bd.nodeClassMask.value = 0;
                  bd.nodeClassMask.bits.Object = 1;
                  bd.nodeClassMask.bits.Variable = 1;
                  bd.nodeId = r->nodeId.nodeId;
                  nextLevelRequest->nodesToBrowse.push_back(bd);
                }
              }

              if (!iter->continuationPoint.empty())
              {
                browseNextRequest->continuationPoints.push_back(iter->continuationPoint);
                browseNextNodeIds.push_back(reqIter->nodeId);
              }
            }

            if (!browseNextRequest->continuationPoints.empty())
            {
              numberOfRequests++;
              browseNextRequest->context = std::make_pair(level, browseNextNodeIds);
              connection->send(browseNextRequest, browseNextCallbackFunction);
            }

            if (!nextLevelRequest->nodesToBrowse.empty())
            {
              numberOfRequests++;
              connection->send(nextLevelRequest, cb);
            }
          }

          numberOfResponses++;
          return true;
        };

        cb = callbackFunction;
        cbbn = browseNextCallbackFunction;

        int level = 0;
        browseRequest->context = level;

        numberOfRequests++;

        // Send the request asynchronously.
        connection->send(browseRequest, callbackFunction);

        // Wait until browsing is complete:
        while (numberOfResponses != numberOfRequests)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "Recursive browsing completed in " << DateTime::diffInMilliseconds(startTime, DateTime(true)).count()
        << " ms, browsed " << totalNodes << " nodes with max. depth " << maxLevel << std::endl;

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

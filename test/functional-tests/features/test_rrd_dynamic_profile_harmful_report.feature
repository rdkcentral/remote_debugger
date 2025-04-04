Feature: Remote Debugger Harmful Commands Dynamic Issuetype Report

  Scenario: Verify remote debugger process is running
    Given the remote debugger process is not running
    When I start the remote debugger process
    Then the remote debugger process should be running

  Scenario: Send WebPA event for Issuetype Test and verify logs
    Given the remote debugger is running
    When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
    Then the logs should contain "SUCCESS: Message sending Done"
    Then the logs should be seen with "SUCCESS: Message Reception Done"
    And the issuetype request should match between Send and Receive

  Scenario: Verify the Issuetype is not found in static profile
    Given the remote debugger received the message from RBUS command
    When the remotedebugger static json profile is present
    Then remotedebugger should read the Json file
    And remotedebugger logs should contain the Json File Parse Success
    And remotedebugger should log as the Issue requested is not found in the profile
    
  Scenario: Verify the Issuetype in dynamic path
    Given the remote debugger issuetype is missing in static profile
    When the remotedebugger read the json file form the dynamic path
    Then remotedebugger json read and parse should be success
    And remotedebugger should read the Issuetype from dynamic profile
    
  Scenario: Check for harmfull commands and abort
    Given remote debugger parse the Dynamic json profile successfully
    When the issue node and subnode are present in the profile
    Then the remote debugger should read the Sanity Check list from profile
    And the remotedebugger should perform sanity check on issue commands
    Given the remote debugger profile has the harmfull commands
    When the issue command and the sanity commands are matched
    Then the remote debugger should exit the processing of commands
    And Abort the commmand execution and skip report upload

Feature: Remote Debugger Background Command Static Report

  Scenario: Check if remote debugger configuration file exists
    Given the configuration file path is set
    When I check if the configuration file exists
    Then the configuration file should exist

  Scenario: Check if /tmp/rrd output directory exists
    Given the /tmp/rrd directory path is set
    When I check if the /tmp/rrd directory exists
    Then the /tmp/rrd directory should exist

  Scenario: Get the current issue type Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType
    When I get the current issue type using rbuscli
    Then the command should execute successfully
    And I should receive the current issue type

  Scenario: Verify remote debugger process is running
    Given the remote debugger process is not running
    When I start the remote debugger process
    Then the remote debugger process should be running

  Scenario: Send WebPA event for Device.Dump Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType
    Given the remote debugger is running
    When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
    Then the event for RRD_SET_ISSUE_EVENT should be received
    And the logs should contain "SUCCESS: Message sending Done"
    And the logs should be seen with "SUCCESS: Message Reception Done"
    When the remotedebugger received the message from webPA event
    Then remotedebugger should read the Json file
    And remotedebugger logs should contain the Json File Parse success
    And the issue data node and sub-node should be found in the JSON file
    And the directory should be created to store the executed output
    And Sanity check to validate the commands should be executed
    And Command output should be added to the output file
    And the issuetype systemd service should start successfully
    And the journalctl service should start successfully
    And the process should sleep with timeout
    And the issuetype systemd service should stop successfully
    And the remotedebugger should call script to upload the debug report

  Scenario: Upload remote debugger debug report
    When I check the upload status in the logs
    Then the upload should be successful if upload is success
    Or the upload should fail if upload fails

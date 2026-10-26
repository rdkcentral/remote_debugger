# RRD Functional Testcases


| Functional Testcases List |
|---|
| Static Profile Debug Report |
| Enable the RRD via TR-181 |
| Disable the RRD via TR-181 |
| Enable RRD via RFC and persist after reboot |
| Validate RRD Debug Report update using RBUS and TR-181 commands |
| Report upload after running the requested Issue command |
| Dynamic Profile Debug Report |
| Append static commands from the dynamic profile for the same IssueType |
| Deep-sleep Profile Debug Report including WebConfig/IARM events |
| Set/Get category-level query from the static JSON profile |
| Set/Get all data from the static JSON profile |
| Support Issue Debug Report requests with UUID |
| Support Issue Debug Report requests without UUID |
| Use the request UUID for report upload after command execution |
| Confirm report upload to the S3 portal |
| Match the request UUID with the uploaded report tarball |
| Execute a valid static IssueType debug command |
| Verify all static IssueTypes |
| Attempt to execute a forbidden command |
| Send a malformed or empty command |
| Download a dynamic debug profile via RDM |
| Trigger a non-existent or misspelled IssueType |
| Validate log/tarball creation and upload |
| Simulate S3 failure and check retry/logging |
| Try running sensitive system commands |
| Send a debug command while in deep sleep |
| Validate resume on wake |
| Flood remote debugger with commands (stress test) |
| Delete/corrupt configuration and simulate network loss |
| Run debugger alongside other system daemons |
| Firmware upgrade and disk stress |
| Set/Get with a misspelled category |
| Set/Get with an empty category |

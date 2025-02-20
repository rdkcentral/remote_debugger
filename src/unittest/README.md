# Remote Debugger GTest

This project contains Google Test (GTest) unit tests for the Remote Debugger component.

## Building the Tests

The project uses a `Makefile` for building the tests. Here are the steps to build and run the tests:

1. Clone the RDK Remote Debugger Repo from RDKE 
    ```
    git clone git@github.com:rdk-e/remote_debugger.git
    ```

2. Checkout to target branch (optional)

3. Navigate to the `src/unittest/` directory.

    ```
    cd ./src/unittest/
    ```

4. Generate the `configure` script.

    ```
    automake --add-missing
    autoreconf --install
    ```

3. Run the `configure` script.

    ```
    ./configure
    ```

4. Build the tests.

    ```
    make
    ```

## Running the Tests

After building the tests, you can run them with running GTest binary `./remotedebugger_gtest`.

This will run all the tests and display the results in the console.

## Generating Code Coverage Report

The project is configured to generate code coverage data using LCOV. Here are the steps to generate a code coverage report:

1. Clean the project.

    ```
    make clean
    ```

2. Remove any existing coverage data files (if any).

    ```
    find . -name "*.gcda" -o -name "*.gcno" -o -name "*.gcov" -exec rm -f {} +
    rm -rf out
    ```

3. Build and run the tests.

    ```
    make
    ./remotedebugger_gtest
    ```

4. Generate the coverage data.

    ```
    lcov --capture --directory . --output-file coverage.info
    ```

5. Filter out system headers from the coverage data.

    ```
    lcov --remove coverage.info '/usr/' --output-file coverage.filtered.info
    ```

6. Generate the coverage report.

    ```
    genhtml coverage.filtered.info --output-directory out
    ```

The coverage report can be viewed by opening the `index.html` file in the `out` directory.

Note: The file structure and organization might differ based on the GTest implementation
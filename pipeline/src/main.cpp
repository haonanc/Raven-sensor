///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2018, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

// Neal Haonan Chen (hc4pa@virginia.edu)
// 6/12/2018

// ZED includes
#include <sl/Camera.hpp>

// Sample includes
#include "utils.hpp"

// Using namespace
using namespace sl;
void convert(sl::Camera &zed, string output_path);
int main(int argc, char **argv) {

    if (argc != 2) {
        std::cout << "Only the path of the output images file should be passed as argument.\n";
        return 1;
    }

    // Create a ZED camera
    Camera zed;

    // Set configuration parameters for the ZED
    InitParameters initParameters;
    initParameters.camera_resolution = RESOLUTION_HD720;
    initParameters.depth_mode = DEPTH_MODE_NONE;

    // Open the camera
    ERROR_CODE err = zed.open(initParameters);
    if (err != SUCCESS) {
        std::cout << toString(err) << std::endl;
        zed.close();
        return 1; // Quit if an error occurred
    }

    // Enable recording with the filename specified in argument
    String path_output("output/");
    err = zed.enableRecording(path_output, SVO_COMPRESSION_MODE_LOSSY);

    if (err != SUCCESS) {
        std::cout << "Recording initialization error. " << toString(err) << std::endl;
        if (err == ERROR_CODE_SVO_RECORDING_ERROR) std::cout << " Note : This error mostly comes from a wrong path or missing writing permissions." << std::endl;
        zed.close();
        return 1;
    }

    // Start recording SVO, stop with Ctrl-C command
    std::cout << "SVO is Recording, use Ctrl-C to stop." << std::endl;
    SetCtrlHandler();
    int frames_recorded = 0;

    while (!exit_app) {
        if (zed.grab() == SUCCESS) {
            // Each new frame is added to the SVO file
            zed.record();
            convert(zed,path_output);
            frames_recorded++;
            std::cout << "Frame count: " << frames_recorded << "\r";
        }
    }

    // Stop recording
    zed.disableRecording();
    zed.close();
    return 0;
}

void convert(sl::Camera &zed, string output_path) {
 
    if (argc != 2 ) {
        cout << " - output file path or image sequence folder(output) : \"path/to/output/file.avi\" or \"path/to/output/folder\"\n";
        cin.ignore();
        return 1;
    }

    // Get input parameters
    string svo_output_path(argv[1]);

    if (!directoryExists(output_path)) {
        cout << "Input directory doesn't exist. Check permissions or create it.\n" << output_path << "\n";
        return 1;
    }

    if(output_path.back() != '/' && output_path.back() != '\\') {
        cout << "Output folder needs to end with '/' or '\\'.\n" << output_path << "\n";
        return 1;
    }

    // Create ZED objects
    Camera zed;

    // Specify SVO path parameter
    InitParameters initParameters;
    initParameters.svo_input_filename.set(svo_input_path.c_str());
    initParameters.coordinate_units = UNIT_MILLIMETER;

    // Open the SVO file specified as a parameter
    ERROR_CODE err = zed.open(initParameters);
    if (err != SUCCESS) {
        cout << toString(err) << endl;
        zed.close();
        return 1; // Quit if an error occurred
    }

    // Get image size
    Resolution image_size = zed.getResolution();
    int width = image_size.width;
    int height = image_size.height;
    int width_sbs = image_size.width * 2;

    // Prepare side by side image containers
    cv::Size image_size_sbs(width_sbs, height); // Size of the side by side image
    cv::Mat svo_image_sbs_rgba(image_size_sbs, CV_8UC4); // Container for ZED RGBA side by side image
    cv::Mat ocv_image_sbs_rgb(image_size_sbs, CV_8UC3); // Container for OpenCV RGB side by side image

    Mat left_image(width, height, MAT_TYPE_8U_C4);
    cv::Mat left_image_ocv = slMat2cvMat(left_image);

    Mat  right_image(width, height, MAT_TYPE_8U_C4);
    cv::Mat right_image_ocv = slMat2cvMat(right_image);

    Mat  depth_image(width, height, MAT_TYPE_32F_C1);
    cv::Mat depth_image_ocv = slMat2cvMat(depth_image);

    RuntimeParameters rt_param;
    rt_param.sensing_mode = SENSING_MODE_FILL;

    // Start SVO conversion to AVI/SEQUENCE
    cout << "ConvertING... Use Ctrl-C to interrupt conversion." << endl;

    int nb_frames = zed.getSVONumberOfFrames();
    int svo_position = 0;

    SetCtrlHandler();

    while (!exit_app) {
        if (zed.grab(rt_param) == SUCCESS) {
            svo_position = zed.getSVOPosition();

            // Retrieve SVO images

            zed.retrieveImage(right_image, VIEW_DEPTH);
            ostringstream filename1;
            filename1 << output_path << "/left" << setfill('0') << setw(6) << svo_position << ".png";
            ostringstream filename2;
            filename2 << output_path << ("/right":"/depth") << setfill('0') << setw(6) << svo_position << ".png";
                
                // Save Left images
            cv::imwrite(filename1.str(), left_image_ocv);
            cv::imwrite(filename2.str(), right_image_ocv);
            ProgressBar((float) (svo_position / (float) nb_frames), 30);

            // Check if we have reached the end of the video
            if (svo_position >= (nb_frames - 1)) { // End of SVO
                cout << "\nSVO end has been reached. Exiting now.\n";
                exit_app = true;
            }
        }
    }

    zed.close();
}



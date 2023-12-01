///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// ACADEMIC OR NON-PROFIT ORGANIZATION NONCOMMERCIAL RESEARCH USE ONLY
//
// BY USING OR DOWNLOADING THE SOFTWARE, YOU ARE AGREEING TO THE TERMS OF THIS LICENSE AGREEMENT.
// IF YOU DO NOT AGREE WITH THESE TERMS, YOU MAY NOT USE OR DOWNLOAD THE SOFTWARE.
//
// License can be found in OpenFace-license.txt

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace 2.0: Facial Behavior Analysis Toolkit
//       Tadas Baltru�aitis, Amir Zadeh, Yao Chong Lim, and Louis-Philippe Morency
//       in IEEE International Conference on Automatic Face and Gesture Recognition, 2018
//
//       Convolutional experts constrained local model for facial landmark detection.
//       A. Zadeh, T. Baltru�aitis, and Louis-Philippe Morency,
//       in Computer Vision and Pattern Recognition Workshops, 2017.
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltru�aitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling
//       in IEEE International. Conference on Computer Vision (ICCV),  2015
//
//       Cross-dataset learning and person-specific normalisation for automatic Action Unit detection
//       Tadas Baltru�aitis, Marwa Mahmoud, and Peter Robinson
//       in Facial Expression Recognition and Analysis Challenge,
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015
//
///////////////////////////////////////////////////////////////////////////////
// FaceLandmarkImg.cpp : Defines the entry point for the console application for detecting landmarks in images.

// dlib
#include <dlib/image_processing/frontal_face_detector.h>

#include "LandmarkCoreIncludes.h"

#include <FaceAnalyser.h>
#include <GazeEstimation.h>

#include <ImageCapture.h>
#include <Visualizer.h>
#include <VisualizationUtils.h>
#include <RecorderOpenFace.h>
#include <RecorderOpenFaceParameters.h>

// zmq
#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/ssl_socket.h>
#include <rabbitmq-c/export.h>
#include <rabbitmq-c/framing.h>
#include <rabbitmq-c/tcp_socket.h>

#include "utils.h"
#include "base64.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <sys/time.h>

#ifndef CONFIG_DIR
#define CONFIG_DIR "~"
#endif

using namespace std;

std::vector<std::string> get_arguments(int argc, char **argv)
{
  std::vector<std::string> arguments;

  for (int i = 0; i < argc; ++i)
  {
    arguments.push_back(std::string(argv[i]));
  }

  return arguments;
}

string convertToJSON(cv::Rect_<float> roi, vector<pair<string, double>> intensity, vector<pair<string, double>> presence)
{
  std::string json = "{";

  json = json + "\"roi\":{\"x\":" + to_string((int)roi.x) + ",\"y\":" + to_string((int)roi.y) +
         ",\"width\":" + to_string((int)roi.width) + ",\"height\":" + to_string((int)roi.height) + "},";

  json = json + "\"intensity\":{";

  for (int i = 0; i < intensity.size(); i++)
  {
    json = json + "\"" + intensity[i].first + "\":" + to_string(intensity[i].second);

    if (i < intensity.size() - 1)
    {
      json = json + ",";
    }
  }

  json = json + "},\"presence\":{";

  for (int i = 0; i < presence.size(); i++)
  {
    json = json + "\"" + presence[i].first + "\":" + to_string(presence[i].second);

    if (i < presence.size() - 1)
    {
      json = json + ",";
    }
  }

  json = json + "}}";

  return json;
}

string convertToJSON(cv::Rect_<float> roi)
{
  std::string json = "{";

  json = json + "\"roi\":{\"x\":" + to_string((int)roi.x) + ",\"y\":" + to_string((int)roi.y) +
         ",\"width\":" + to_string((int)roi.width) + ",\"height\":" + to_string((int)roi.height) + "},";

  json = json + "\"intensity\":{\"AU01\":\"-\",\"AU02\":\"-\",\"AU04\":\"-\",\"AU05\":\"-\"," +
         "\"AU06\":\"-\",\"AU07\":\"-\",\"AU09\":\"-\",\"AU10\":\"-\",\"AU12\":\"-\",\"AU14\":\"-\"," +
         "\"AU15\":\"-\",\"AU17\":\"-\",\"AU20\":\"-\",\"AU23\":\"-\",\"AU25\":\"-\",\"AU26\":\"-\",\"AU45\":\"-\"},";

  json = json + "\"presence\":{\"AU01\":\"-\",\"AU02\":\"-\",\"AU04\":\"-\",\"AU05\":\"-\",\"AU06\":\"-\",\"AU07\":\"-\"," +
         "\"AU09\":\"-\",\"AU10\":\"-\",\"AU12\":\"-\",\"AU14\":\"-\",\"AU15\":\"-\",\"AU17\":\"-\",\"AU20\":\"-\"," +
         "\"AU23\":\"-\",\"AU25\":\"-\",\"AU26\":\"-\",\"AU28\":\"-\",\"AU45\":\"-\"}";

  json = json + "}";

  return json;
}

string convertToJSON()
{
  std::string json = "{";

  json = json + "\"intensity\":{\"AU01\":\"-\",\"AU02\":\"-\",\"AU04\":\"-\",\"AU05\":\"-\"," +
         "\"AU06\":\"-\",\"AU07\":\"-\",\"AU09\":\"-\",\"AU10\":\"-\",\"AU12\":\"-\",\"AU14\":\"-\"," +
         "\"AU15\":\"-\",\"AU17\":\"-\",\"AU20\":\"-\",\"AU23\":\"-\",\"AU25\":\"-\",\"AU26\":\"-\",\"AU45\":\"-\"},";

  json = json + "\"presence\":{\"AU01\":\"-\",\"AU02\":\"-\",\"AU04\":\"-\",\"AU05\":\"-\",\"AU06\":\"-\",\"AU07\":\"-\"," +
         "\"AU09\":\"-\",\"AU10\":\"-\",\"AU12\":\"-\",\"AU14\":\"-\",\"AU15\":\"-\",\"AU17\":\"-\",\"AU20\":\"-\"," +
         "\"AU23\":\"-\",\"AU25\":\"-\",\"AU26\":\"-\",\"AU28\":\"-\",\"AU45\":\"-\"}";

  json = json + "}";

  return json;
}

void loadFaceModel(LandmarkDetector::CLNF &clnf_model, LandmarkDetector::FaceModelParameters &params)
{
  if (clnf_model.face_detector_HAAR.empty() && params.curr_face_detector == params.HAAR_DETECTOR)
  {
    clnf_model.face_detector_HAAR.load(params.haar_face_detector_location);
    clnf_model.haar_face_detector_location = params.haar_face_detector_location;
  }

  if (clnf_model.face_detector_MTCNN.empty() && params.curr_face_detector == params.MTCNN_DETECTOR)
  {
    clnf_model.face_detector_MTCNN.Read(params.mtcnn_face_detector_location);
    clnf_model.mtcnn_face_detector_location = params.mtcnn_face_detector_location;

    // If the model is still empty default to HOG
    if (clnf_model.face_detector_MTCNN.empty())
    {
      std::cout << "INFO: defaulting to HOG-SVM face detector" << std::endl;
      params.curr_face_detector = LandmarkDetector::FaceModelParameters::HOG_SVM_DETECTOR;
    }
  }

  // Warm up the models
  std::cout << "Warming up the models..." << std::endl;
  cv::Mat dummy_image = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
  for (int i = 0; i < 10; ++i)
  {
    LandmarkDetector::DetectLandmarksInVideo(dummy_image, clnf_model, params, dummy_image);
  }
  std::cout << "Warm up runs are completed" << std::endl;

  // Reset the model parameters
  clnf_model.Reset();
}

cv::Rect_<float> detectSingleFace(const cv::Mat &rgb_image, LandmarkDetector::CLNF &clnf_model, LandmarkDetector::FaceModelParameters &params, cv::Mat &grayscale_image)
{
  cv::Rect_<float> bounding_box(0, 0, 0, 0);

  if (params.curr_face_detector == LandmarkDetector::FaceModelParameters::HOG_SVM_DETECTOR)
  {
    float confidence;
    LandmarkDetector::DetectSingleFaceHOG(bounding_box, grayscale_image, clnf_model.face_detector_HOG, confidence);
  }
  else if (params.curr_face_detector == LandmarkDetector::FaceModelParameters::HAAR_DETECTOR)
  {
    LandmarkDetector::DetectSingleFace(bounding_box, rgb_image, clnf_model.face_detector_HAAR);
  }
  else if (params.curr_face_detector == LandmarkDetector::FaceModelParameters::MTCNN_DETECTOR)
  {
    float confidence;
    LandmarkDetector::DetectSingleFaceMTCNN(bounding_box, rgb_image, clnf_model.face_detector_MTCNN, confidence);
  }

  // Add some buffer for the bounding_box
  int buffer = max(bounding_box.width, bounding_box.height) / 4;
  bounding_box.x = bounding_box.x - buffer;
  bounding_box.y = bounding_box.y - buffer;
  bounding_box.width = bounding_box.width + (2 * buffer);
  bounding_box.height = bounding_box.height + (2 * buffer);

  // Cast bounding_box to integer
  bounding_box.x = (int)bounding_box.x - 1;
  bounding_box.y = (int)bounding_box.y - 1;
  bounding_box.width = (int)bounding_box.width + 2;
  bounding_box.height = (int)bounding_box.height + 2;

  // Check image boundaries
  bounding_box.x = bounding_box.x > 0 ? bounding_box.x : 0;
  bounding_box.y = bounding_box.y > 0 ? bounding_box.y : 0;
  bounding_box.width = (bounding_box.x + bounding_box.width) < rgb_image.size().width ? bounding_box.width : (rgb_image.size().width - bounding_box.x);
  bounding_box.height = (bounding_box.y + bounding_box.height) < rgb_image.size().height ? bounding_box.height : (rgb_image.size().height - bounding_box.y);

  return bounding_box;
}

int main(int argc, char **argv)
{
  // Convert arguments to more convenient vector form
  std::vector<std::string> arguments = get_arguments(argc, argv);
  if (arguments.size() <= 1)
  {
    std::cout << "Please add argument for the queue name, e.g. the session ID of the experiment" << std::endl;
    return -1;
  }

  // Load the models
  LandmarkDetector::FaceModelParameters det_parameters(arguments);

  // The modules that are being used for tracking
  std::cout << "Loading landmark model" << std::endl;
  LandmarkDetector::CLNF face_model(det_parameters.model_location);

  loadFaceModel(face_model, det_parameters);

  if (!face_model.loaded_successfully)
  {
    std::cout << "ERROR: Could not load the landmark detector" << std::endl;
    return 1;
  }

  std::cout << "Loading facial feature extractor" << std::endl;
  FaceAnalysis::FaceAnalyserParameters face_analysis_params(arguments);
  face_analysis_params.OptimizeForImages();
  FaceAnalysis::FaceAnalyser face_analyser(face_analysis_params);

  std::cout << "Everything loaded" << std::endl;

  // TODO: make this configurable, e.g. read from config file
  char const *hostname = "localhost";
  int port = 5672;
  char const *exchange = "amq.direct";
  amqp_socket_t *socket = NULL;
  amqp_connection_state_t conn = amqp_new_connection();

  socket = amqp_tcp_socket_new(conn);
  if (!socket)
  {
    die("Creating TCP socket");
  }

  die_on_error(amqp_socket_open_noblock(socket, hostname, port, 0),
               "Opening TCP socket");

  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                               "guest", "guest"),
                    "Logging in");
  amqp_channel_open(conn, 1);
  amqp_get_rpc_reply(conn);
  std::cout << "Opening channel" << std::endl;

  amqp_bytes_t queuename = amqp_cstring_bytes(arguments[1].c_str());

  amqp_basic_qos(conn, 1, 0, 10, 0);
  amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0,
                     amqp_empty_table);
  amqp_get_rpc_reply(conn);

  cv::Rect_<float> roi(0, 0, 0, 0);
  cv::Mat greyScale_image, rgb_image_roi, greyScale_image_roi;
  int original_frame_width = -1;
  int original_frame_height = -1;
  bool init_roi = true;

  // TODO: remove these since they are only used for debugging
  int frame_count = 0;
  struct timeval stop, start, stop_all, start_all;

  while (true)
  {
    amqp_rpc_reply_t res;
    amqp_envelope_t envelope;
    cv::Mat greyScale_image;

    amqp_maybe_release_buffers(conn);

    res = amqp_consume_message(conn, &envelope, NULL, 0);

    if (AMQP_RESPONSE_NORMAL != res.reply_type)
    {
      break;
    }

    printf("Delivery %u, exchange %.*s routingkey %.*s\n",
           (unsigned)envelope.delivery_tag, (int)envelope.exchange.len,
           (char *)envelope.exchange.bytes, (int)envelope.routing_key.len,
           (char *)envelope.routing_key.bytes);

    if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG)
    {
      printf("Content-type: %.*s\n",
             (int)envelope.message.properties.content_type.len,
             (char *)envelope.message.properties.content_type.bytes);
    }

    amqp_bytes_t msg = amqp_bytes_malloc_dup(envelope.message.body);

    std::string img_str = std::string(static_cast<char *>(msg.bytes), msg.len);

    gettimeofday(&start_all, NULL);

    gettimeofday(&start, NULL);

    // decode image
    std::string dec_jpg;
    try
    {
      dec_jpg = base64_decode(img_str);
    }
    catch (const std::exception &e)
    {
      printf("Not valid encoded base64 image\n");
      continue;
    }
    std::vector<uchar> data(dec_jpg.begin(), dec_jpg.end());
    cv::Mat rgb_image = cv::imdecode(cv::Mat(data), 1);
    cv::cvtColor(rgb_image, greyScale_image, cv::COLOR_BGR2GRAY);

    gettimeofday(&stop, NULL);
    printf("Decode: %f\n", (double)((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000000);

    if (original_frame_width == -1 && original_frame_height == -1)
    {
      original_frame_width = rgb_image.size().width;
      original_frame_height = rgb_image.size().height;
    }

    // Calculate ROI
    if (init_roi)
    {
      roi = detectSingleFace(rgb_image, face_model, det_parameters, greyScale_image);
      init_roi = false;
    }

    if (roi.width > 2 && rgb_image.size().width == original_frame_width && rgb_image.size().height == original_frame_height)
    {
      // Crop the RGB and GrayScale frame based on ROI
      rgb_image_roi = rgb_image(roi);
      greyScale_image_roi = greyScale_image(roi);
    }
    else
    {
      rgb_image_roi = rgb_image;
      greyScale_image_roi = greyScale_image;
    }

    std::cout << "Image size: " << rgb_image.size() << " -> " << rgb_image_roi.size() << std::endl;

    gettimeofday(&start, NULL);

    // results will be stored in face_model
    bool landmark_detection_success = LandmarkDetector::DetectLandmarksInVideo(rgb_image_roi, face_model, det_parameters, greyScale_image_roi);

    gettimeofday(&stop, NULL);
    printf("DetectLandmarksInVideo: %f\n", (double)((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000000);

    string json;
    if (landmark_detection_success)
    {
      gettimeofday(&start, NULL);

      face_analyser.PredictStaticAUsAndComputeFeatures(rgb_image, face_model.detected_landmarks);

      auto aus_intensity = face_analyser.GetCurrentAUsReg();
      auto aus_presence = face_analyser.GetCurrentAUsClass();

      json = convertToJSON(roi, aus_intensity, aus_presence);

      gettimeofday(&stop, NULL);
      printf("PredictStaticAUsAndComputeFeatures & convertToJSON: %f\n", (double)((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000000);
    }
    else
    {
      // Tracking or detection failed; so reset the model and set ROI to 0 in order to receive full frame from ZMQ
      if (
          (!face_model.tracking_initialised && (face_model.failures_in_a_row + 1) % (det_parameters.reinit_video_every * 6) == 0) || (face_model.tracking_initialised && !face_model.detection_success && det_parameters.reinit_video_every > 0 && face_model.failures_in_a_row % det_parameters.reinit_video_every == 0))
      {
        std::cout << "RESET" << std::endl;

        init_roi = true;

        face_model.Reset();

        json = convertToJSON();
      }
      else
      {
        json = convertToJSON(roi);
      }
    }

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CORRELATION_ID_FLAG;
    props.correlation_id = envelope.message.properties.correlation_id;

    printf("Reply-to-queue: %s\n", (char *)envelope.message.properties.reply_to.bytes);

    die_on_error(amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange), envelope.message.properties.reply_to, 0, 0,
                                    &props, amqp_cstring_bytes(json.c_str())),
                 "Publishing");

    printf("Finish publish reply: %s\n", json.c_str());

    gettimeofday(&stop_all, NULL);
    printf("[%d] All: %f\n\n", frame_count, (double)((stop_all.tv_sec - start_all.tv_sec) * 1000000 + stop_all.tv_usec - start_all.tv_usec) / 1000000);

    // Save image for debugging purposes
    // for(int j = 0; j < face_model.detected_landmarks.rows / 2; j++){
    // 	float x = face_model.detected_landmarks[j][0];
    // 	float y = face_model.detected_landmarks[j + face_model.detected_landmarks.rows / 2][0];
    // 	cv::circle(rgb_image_roi, cv::Point2f(x,y), 4, cv::Scalar(255, 0, 0), cv::FILLED, cv::LINE_8);
    // }
    // cv::imwrite("../../experimental-hub-openface-test/" + to_string(frame_count) + ".jpg", rgb_image_roi);
    amqp_bytes_free(msg);

    amqp_destroy_envelope(&envelope);
    frame_count++;
  }

  amqp_bytes_free(queuename);

  amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
  printf("Closing channel\n");
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  printf("Closing connection\n");
  amqp_destroy_connection(conn);
  printf("Ending connection\n");

  return 0;
}

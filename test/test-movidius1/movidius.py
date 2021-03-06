#! /usr/bin/env python3

# Copyright(c) 2017 Intel Corporation. 
# License: MIT See LICENSE file in root directory.

from mvnc import mvncapi as mvnc
import sys
import numpy as np
import cv2

import time
# Assume running in examples/caffe/TinyYolo and graph file is in current directory.
#input_image_file= '../../data/images/nps_chair.png'
#input_image_file= 'test.mp4'
#input_image_file= '/media/216/root/repo-github/test.avi'
input_image_file= '../../test.avi'
#input_image_file= 'rtsp://192.168.1.80:554/av0_1'
#tiny_yolo_graph_file= './graph'
tiny_yolo_graph_file= '/media/216/root/repo-github/graph'

# Tiny Yolo assumes input images are these dimensions.
NETWORK_IMAGE_WIDTH = 448
NETWORK_IMAGE_HEIGHT = 448

TY_NETWORK_IMAGE_WIDTH = 448
TY_NETWORK_IMAGE_HEIGHT = 448
g_fifo_in = []
g_fifo_out = []
g_graph = []
# Interpret the output from a single inference of TinyYolo (GetResult)
# and filter out objects/boxes with low probabilities.
# output is the array of floats returned from the API GetResult but converted
# to float32 format.
# input_image_width is the width of the input image
# input_image_height is the height of the input image
# Returns a list of lists. each of the inner lists represent one found object and contain
# the following 6 values:
#    string that is network classification ie 'cat', or 'chair' etc
#    float value for box center X pixel location within source image
#    float value for box center Y pixel location within source image
#    float value for box width in pixels within source image
#    float value for box height in pixels within source image
#    float value that is the probability for the network classification.
def filter_objects(inference_result, input_image_width, input_image_height):

    # the raw number of floats returned from the inference (GetResult())
    num_inference_results = len(inference_result)

    # the 20 classes this network was trained on
    network_classifications = ["aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car",
                               "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike",
                               "person", "pottedplant", "sheep", "sofa", "train","tvmonitor"]

    # only keep boxes with probabilities greater than this
    probability_threshold = 0.07

    num_classifications = len(network_classifications) # should be 20
    grid_size = 7 # the image is a 7x7 grid.  Each box in the grid is 64x64 pixels
    boxes_per_grid_cell = 2 # the number of boxes returned for each grid cell

    # grid_size is 7 (grid is 7x7)
    # num classifications is 20
    # boxes per grid cell is 2
    all_probabilities = np.zeros((grid_size, grid_size, boxes_per_grid_cell, num_classifications))

    # classification_probabilities  contains a probability for each classification for
    # each 64x64 pixel square of the grid.  The source image contains
    # 7x7 of these 64x64 pixel squares and there are 20 possible classifications
    classification_probabilities = \
        np.reshape(inference_result[0:980], (grid_size, grid_size, num_classifications))
    num_of_class_probs = len(classification_probabilities)

    # The probability scale factor for each box
    box_prob_scale_factor = np.reshape(inference_result[980:1078], (grid_size, grid_size, boxes_per_grid_cell))

    # get the boxes from the results and adjust to be pixel units
    all_boxes = np.reshape(inference_result[1078:], (grid_size, grid_size, boxes_per_grid_cell, 4))
    boxes_to_pixel_units(all_boxes, input_image_width, input_image_height, grid_size)

    # adjust the probabilities with the scaling factor
    for box_index in range(boxes_per_grid_cell): # loop over boxes
        for class_index in range(num_classifications): # loop over classifications
            all_probabilities[:,:,box_index,class_index] = np.multiply(classification_probabilities[:,:,class_index],box_prob_scale_factor[:,:,box_index])


    probability_threshold_mask = np.array(all_probabilities>=probability_threshold, dtype='bool')
    box_threshold_mask = np.nonzero(probability_threshold_mask)
    boxes_above_threshold = all_boxes[box_threshold_mask[0],box_threshold_mask[1],box_threshold_mask[2]]
    classifications_for_boxes_above = np.argmax(all_probabilities,axis=3)[box_threshold_mask[0],box_threshold_mask[1],box_threshold_mask[2]]
    probabilities_above_threshold = all_probabilities[probability_threshold_mask]

    # sort the boxes from highest probability to lowest and then
    # sort the probabilities and classifications to match
    argsort = np.array(np.argsort(probabilities_above_threshold))[::-1]
    boxes_above_threshold = boxes_above_threshold[argsort]
    classifications_for_boxes_above = classifications_for_boxes_above[argsort]
    probabilities_above_threshold = probabilities_above_threshold[argsort]


    # get mask for boxes that seem to be the same object
    duplicate_box_mask = get_duplicate_box_mask(boxes_above_threshold)

    # update the boxes, probabilities and classifications removing duplicates.
    boxes_above_threshold = boxes_above_threshold[duplicate_box_mask]
    classifications_for_boxes_above = classifications_for_boxes_above[duplicate_box_mask]
    probabilities_above_threshold = probabilities_above_threshold[duplicate_box_mask]

    classes_boxes_and_probs = []
    for i in range(len(boxes_above_threshold)):
        classes_boxes_and_probs.append([network_classifications[classifications_for_boxes_above[i]],boxes_above_threshold[i][0],boxes_above_threshold[i][1],boxes_above_threshold[i][2],boxes_above_threshold[i][3],probabilities_above_threshold[i]])

    return classes_boxes_and_probs

# creates a mask to remove duplicate objects (boxes) and their related probabilities and classifications
# that should be considered the same object.  This is determined by how similar the boxes are
# based on the intersection-over-union metric.
# box_list is as list of boxes (4 floats for centerX, centerY and Length and Width)
def get_duplicate_box_mask(box_list):
    # The intersection-over-union threshold to use when determining duplicates.
    # objects/boxes found that are over this threshold will be
    # considered the same object
    max_iou = 0.35

    box_mask = np.ones(len(box_list))

    for i in range(len(box_list)):
        if box_mask[i] == 0: continue
        for j in range(i + 1, len(box_list)):
            if get_intersection_over_union(box_list[i], box_list[j]) > max_iou:
                box_mask[j] = 0.0

    filter_iou_mask = np.array(box_mask > 0.0, dtype='bool')
    return filter_iou_mask

# Converts the boxes in box list to pixel units
# assumes box_list is the output from the box output from
# the tiny yolo network and is [grid_size x grid_size x 2 x 4].
def boxes_to_pixel_units(box_list, image_width, image_height, grid_size):

    # number of boxes per grid cell
    boxes_per_cell = 2

    # setup some offset values to map boxes to pixels
    # box_offset will be [[ [0, 0], [1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6]] ...repeated for 7 ]
    box_offset = np.transpose(np.reshape(np.array([np.arange(grid_size)]*(grid_size*2)),(boxes_per_cell,grid_size, grid_size)),(1,2,0))

    # adjust the box center
    box_list[:,:,:,0] += box_offset
    box_list[:,:,:,1] += np.transpose(box_offset,(1,0,2))
    box_list[:,:,:,0:2] = box_list[:,:,:,0:2] / (grid_size * 1.0)

    # adjust the lengths and widths
    box_list[:,:,:,2] = np.multiply(box_list[:,:,:,2],box_list[:,:,:,2])
    box_list[:,:,:,3] = np.multiply(box_list[:,:,:,3],box_list[:,:,:,3])

    #scale the boxes to the image size in pixels
    box_list[:,:,:,0] *= image_width
    box_list[:,:,:,1] *= image_height
    box_list[:,:,:,2] *= image_width
    box_list[:,:,:,3] *= image_height


# Evaluate the intersection-over-union for two boxes
# The intersection-over-union metric determines how close
# two boxes are to being the same box.  The closer the boxes
# are to being the same, the closer the metric will be to 1.0
# box_1 and box_2 are arrays of 4 numbers which are the (x, y)
# points that define the center of the box and the length and width of
# the box.
# Returns the intersection-over-union (between 0.0 and 1.0)
# for the two boxes specified.
def get_intersection_over_union(box_1, box_2):

    # one diminsion of the intersecting box
    intersection_dim_1 = min(box_1[0]+0.5*box_1[2],box_2[0]+0.5*box_2[2])-\
                         max(box_1[0]-0.5*box_1[2],box_2[0]-0.5*box_2[2])

    # the other dimension of the intersecting box
    intersection_dim_2 = min(box_1[1]+0.5*box_1[3],box_2[1]+0.5*box_2[3])-\
                         max(box_1[1]-0.5*box_1[3],box_2[1]-0.5*box_2[3])

    if intersection_dim_1 < 0 or intersection_dim_2 < 0 :
        # no intersection area
        intersection_area = 0
    else :
        # intersection area is product of intersection dimensions
        intersection_area =  intersection_dim_1*intersection_dim_2

    # calculate the union area which is the area of each box added
    # and then we need to subtract out the intersection area since
    # it is counted twice (by definition it is in each box)
    union_area = box_1[2]*box_1[3] + box_2[2]*box_2[3] - intersection_area;

    # now we can return the intersection over union
    iou = intersection_area / union_area

    return iou
def get_objects(source_image, filtered_objects):
    # copy image so we can draw on it. Could just draw directly on source image if not concerned about that.
    display_image = source_image.copy()
    source_image_width = source_image.shape[1]
    source_image_height = source_image.shape[0]

    x_ratio = float(source_image_width) / NETWORK_IMAGE_WIDTH
    y_ratio = float(source_image_height) / NETWORK_IMAGE_HEIGHT

    # loop through each box and draw it on the image along with a classification label
    print('Found this many objects in the image: ' + str(len(filtered_objects)))
    objs=[];
    for obj_index in range(len(filtered_objects)):
        center_x = int(filtered_objects[obj_index][1] * x_ratio)
        center_y = int(filtered_objects[obj_index][2] * y_ratio)
        half_width = int(filtered_objects[obj_index][3] * x_ratio)//2
        half_height = int(filtered_objects[obj_index][4] * y_ratio)//2

        # calculate box (left, top) and (right, bottom) coordinates
        box_left = max(center_x - half_width, 0)
        box_top = max(center_y - half_height, 0)
        box_right = min(center_x + half_width, source_image_width)
        box_bottom = min(center_y + half_height, source_image_height)
        objs=objs+[box_left,box_top,box_right,box_bottom]
    return objs
        
# Displays a gui window with an image that contains
# boxes and lables for found objects.  will not return until
# user presses a key.
# source_image is the original image for the inference before it was resized or otherwise changed.
# filtered_objects is a list of lists (as returned from filter_objects()
# each of the inner lists represent one found object and contain
# the following 6 values:
#    string that is network classification ie 'cat', or 'chair' etc
#    float value for box center X pixel location within source image
#    float value for box center Y pixel location within source image
#    float value for box width in pixels within source image
#    float value for box height in pixels within source image
#    float value that is the probability for the network classification.
def display_objects_in_gui(source_image, filtered_objects):
    # copy image so we can draw on it. Could just draw directly on source image if not concerned about that.
    display_image = source_image.copy()
    source_image_width = source_image.shape[1]
    source_image_height = source_image.shape[0]

    x_ratio = float(source_image_width) / NETWORK_IMAGE_WIDTH
    y_ratio = float(source_image_height) / NETWORK_IMAGE_HEIGHT

    # loop through each box and draw it on the image along with a classification label
    print('Found this many objects in the image: ' + str(len(filtered_objects)))
    for obj_index in range(len(filtered_objects)):
        center_x = int(filtered_objects[obj_index][1] * x_ratio)
        center_y = int(filtered_objects[obj_index][2] * y_ratio)
        half_width = int(filtered_objects[obj_index][3] * x_ratio)//2
        half_height = int(filtered_objects[obj_index][4] * y_ratio)//2

        # calculate box (left, top) and (right, bottom) coordinates
        box_left = max(center_x - half_width, 0)
        box_top = max(center_y - half_height, 0)
        box_right = min(center_x + half_width, source_image_width)
        box_bottom = min(center_y + half_height, source_image_height)

   #     print('box at index ' + str(obj_index) + ' is... left: ' + str(box_left) + ', top: ' + str(box_top) + ', right: ' + str(box_right) + ', bottom: ' + str(box_bottom))

        #draw the rectangle on the image.  This is hopefully around the object
        box_color = (0, 255, 0)  # green box
        box_thickness = 2
        cv2.rectangle(display_image, (box_left, box_top),(box_right, box_bottom), box_color, box_thickness)

        # draw the classification label string just above and to the left of the rectangle
        label_background_color = (70, 120, 70) # greyish green background for text
        label_text_color = (255, 255, 255)   # white text
        cv2.rectangle(display_image,(box_left, box_top-20),(box_right,box_top), label_background_color, -1)
        cv2.putText(display_image,filtered_objects[obj_index][0] + ' : %.2f' % filtered_objects[obj_index][5], (box_left+5,box_top-7), cv2.FONT_HERSHEY_SIMPLEX, 0.5, label_text_color, 1)

    window_name = 'TinyYolo (hit key to exit)'
    cv2.imshow(window_name, display_image)
    while (True):
        raw_key = cv2.waitKey(10)
        # check if the window is visible, this means the user hasn't closed
        # the window via the X button (may only work with opencv 3.x
        prop_val = cv2.getWindowProperty(window_name, cv2.WND_PROP_ASPECT_RATIO)
        #if ((raw_key != -1) or (prop_val < 0.0)):
            # the user hit a key or closed the window (in that order)
        break





def main_test():
    print('Running NCS Caffe TinyYolo example')

    # Set logging level to only log errors
    mvnc.global_set_option(mvnc.GlobalOption.RW_LOG_LEVEL, 3)
    devices = mvnc.enumerate_devices()
    if len(devices) == 0:
        print('No devices found')
        return 1
    device = mvnc.Device(devices[0])
    device.open()

    #Load graph from disk and allocate graph via API
    with open(tiny_yolo_graph_file, mode='rb') as f:
        graph_from_disk = f.read()
    graph = mvnc.Graph("Tiny Yolo Graph")
    fifo_in, fifo_out = graph.allocate_with_fifos(device, graph_from_disk)

    # Read image from file, resize it to network width and height
    # save a copy in display_image for display, then convert to float32, normalize (divide by 255),
    # and finally convert to convert to float16 to pass to LoadTensor as input for an inference
    cv_window_name='123'

    while (True):
        video_device = cv2.VideoCapture("./" + input_image_file)
        actual_frame_width = video_device.get(cv2.CAP_PROP_FRAME_WIDTH)
        actual_frame_height = video_device.get(cv2.CAP_PROP_FRAME_HEIGHT)
        print ('actual video resolution: ' + str(actual_frame_width) + ' x ' + str(actual_frame_height))
#        if ((video_device == None) or (not video_device.isOpened())):
#            print ('Could not open video device.  Make sure file exists:')
#            print ('file name:' + input_video_file)
#            print ('Also, if you installed python opencv via pip or pip3 you')
#            print ('need to uninstall it and install from source with -D WITH_V4L=ON')
#            print ('Use the provided script: install-opencv-from_source.sh')

       # frame_count = 0
       # start_time = time.time()

#frame_count = frame_count + 1

#frames_per_second = frame_count / (end_time - start_time)


        while True :
            print ('Use the provided script: install-opencv-from_source.sh')

# Read image from video device,
            time1  = time.time()
            ret_val, input_image = video_device.read()

            time2  = time.time()

            start_time = time.time()


            display_image = input_image



            input_image = cv2.resize(input_image, (NETWORK_IMAGE_WIDTH, NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)
            input_image = input_image.astype(np.float32)
            input_image = np.divide(input_image, 255.0)
            input_image = input_image[:, :, ::-1]  # convert to RGB

            time3  = time.time()
            # Load tensor and get result.  This executes the inference on the NCS
            graph.queue_inference_with_fifo_elem(fifo_in, fifo_out, input_image.astype(np.float32), None)
            output, userobj = fifo_out.read_elem()

            # filter out all the objects/boxes that don't meet thresholds
            filtered_objs = filter_objects(output.astype(np.float32), input_image.shape[1], input_image.shape[0])

            print('Displaying image with objects detected in GUI')
            print('Click in the GUI window and hit any key to exit')
            #display the filtered objects/boxes in a GUI window
            display_objects_in_gui(display_image, filtered_objs)



            time4  = time.time()










            end_time = time.time()
            time_used=end_time-start_time
            print("time used : "+ str(time_used)+"time point:"+str(time1)+" "+str(time2)+" "+str(time3)+" "+str(time4))
            cv2.imshow(cv_window_name, input_image)
            #raw_key = cv2.waitKey(1)

            if (not ret_val):
             #   end_time = time.time()
                print("No image from from video device, exiting")
                break

             # resize image to network width and height
             # then convert to float32, normalize (divide by 255),
             # and finally convert to float16 to pass to LoadTensor as input
             # for an inference
       # input_image = cv2.resize(input_image, (TY_NETWORK_IMAGE_WIDTH, TY_NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)



             # save a display image as read from video device.

        # close video device
        video_device.release()














    input_image = cv2.imread(input_image_file)
    display_image = input_image



    input_image = cv2.resize(input_image, (NETWORK_IMAGE_WIDTH, NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)
    input_image = input_image.astype(np.float32)
    input_image = np.divide(input_image, 255.0)
    input_image = input_image[:, :, ::-1]  # convert to RGB

    # Load tensor and get result.  This executes the inference on the NCS
    graph.queue_inference_with_fifo_elem(fifo_in, fifo_out, input_image.astype(np.float32), None)
    output, userobj = fifo_out.read_elem()

    # filter out all the objects/boxes that don't meet thresholds
    filtered_objs = filter_objects(output.astype(np.float32), input_image.shape[1], input_image.shape[0])

    print('Displaying image with objects detected in GUI')
    print('Click in the GUI window and hit any key to exit')
    #display the filtered objects/boxes in a GUI window
    display_objects_in_gui(display_image, filtered_objs)

    fifo_in.destroy()
    fifo_out.destroy()
    graph.destroy()
    device.close()
    device.destroy()
    print('Finished')

def init():
    global g_device
    global g_graph
    global g_fifo_in
    global g_fifo_out
    print('init start\n')
    # Set logging level to only log errors
    mvnc.global_set_option(mvnc.GlobalOption.RW_LOG_LEVEL, 3)
    devices = mvnc.enumerate_devices()
    if len(devices) == 0:
        print('No devices found')
        return 1
    g_device = mvnc.Device(devices[0])
    g_device.open()

    #Load graph from disk and allocate graph via API
    with open(tiny_yolo_graph_file, mode='rb') as f:
        graph_from_disk = f.read()
    g_graph = mvnc.Graph("Tiny Yolo Graph")
    g_fifo_in, g_fifo_out = g_graph.allocate_with_fifos(g_device, graph_from_disk)
    print('init end')

def run():
    global g_device
    global g_graph
    global g_fifo_in
    global g_fifo_out      
    print('run start')


    video_device = cv2.VideoCapture("./" + input_image_file)
    actual_frame_width = video_device.get(cv2.CAP_PROP_FRAME_WIDTH)
    actual_frame_height = video_device.get(cv2.CAP_PROP_FRAME_HEIGHT)
    print ('actual video resolution: ' + str(actual_frame_width) + ' x ' + str(actual_frame_height))




   # while(1):
    ret_val, input_image = video_device.read()
    input_image = cv2.resize(input_image, (NETWORK_IMAGE_WIDTH, NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)
    input_image = input_image.astype(np.float32)
    input_image = np.divide(input_image, 255.0)
    input_image = input_image[:, :, ::-1]  # convert to RGB

    time3  = time.time()
    # Load tensor and get result.  This executes the inference on the NCS

    g_graph.queue_inference_with_fifo_elem(g_fifo_in, g_fifo_out, input_image.astype(np.float32), None)
    output, userobj = g_fifo_out.read_elem()

    # filter out all the objects/boxes that don't meet thresholds
    filtered_objs = filter_objects(output.astype(np.float32), input_image.shape[1], input_image.shape[0])

    print('Displaying image with objects detected in GUI')
    print('Click in the GUI window and hit any key to exit')
    #display the filtered objects/boxes in a GUI window
    display_objects_in_gui(input_image, filtered_objs)


    print('run end')
def run_pic(picture):
    global g_device
    global g_graph
    global g_fifo_in
    global g_fifo_out
    print('run start')


    video_device = cv2.VideoCapture("./" + input_image_file)
    actual_frame_width = video_device.get(cv2.CAP_PROP_FRAME_WIDTH)
    actual_frame_height = video_device.get(cv2.CAP_PROP_FRAME_HEIGHT)
    print ('actual video resolution: ' + str(actual_frame_width) + ' x ' + str(actual_frame_height))




   # while(1):
    ret_val, input_image = video_device.read()
    ret_val, input_image = video_device.read()

#    a1=input_image.data();
#    input_image=mat(a1);

    input_image = cv2.resize(input_image, (NETWORK_IMAGE_WIDTH, NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)
    input_image = input_image.astype(np.float32)
    input_image = np.divide(input_image, 255.0)
    input_image = input_image[:, :, ::-1]  # convert to RGB

    time3  = time.time()
    # Load tensor and get result.  This executes the inference on the NCS

    g_graph.queue_inference_with_fifo_elem(g_fifo_in, g_fifo_out, input_image.astype(np.float32), None)
    output, userobj = g_fifo_out.read_elem()

    # filter out all the objects/boxes that don't meet thresholds
    filtered_objs = filter_objects(output.astype(np.float32), input_image.shape[1], input_image.shape[0])

    print('Displaying image with objects detected in GUI')
    print('Click in the GUI window and hit any key to exit')
    #display the filtered objects/boxes in a GUI window
    display_objects_in_gui(input_image, filtered_objs)

def run_1_pic(picture):
    global g_device
    global g_graph
    global g_fifo_in
    global g_fifo_out
#    print('run start\n')
 #   cv2.imshow("111",picture)
 #   cv2.waitKey(10)
 #   return
    video_device = cv2.VideoCapture("./" + input_image_file)
    actual_frame_width = video_device.get(cv2.CAP_PROP_FRAME_WIDTH)
    actual_frame_height = video_device.get(cv2.CAP_PROP_FRAME_HEIGHT)
    print ('actual video resolution: ' + str(actual_frame_width) + ' x ' + str(actual_frame_height))

  #  print('run 1.....')


   # while(1):
#    ret_val, input_image = video_device.read()
    input_image1 = picture
  #  print('run 1.0.0')      
    input_image = picture
 #   a1=input_image.data();
#    input_image=mat(a1);
    input_image = cv2.resize(input_image, (NETWORK_IMAGE_WIDTH, NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)
  #  print('run 1.0')      
    input_image = input_image.astype(np.float32)
 #   print('run 1.1')    
    input_image = np.divide(input_image, 255.0)
    input_image = input_image[:, :, ::-1]  # convert to RGB

    time3  = time.time()
    # Load tensor and get result.  This executes the inference on the NCS
 #   print('run 2')
    g_graph.queue_inference_with_fifo_elem(g_fifo_in, g_fifo_out, input_image.astype(np.float32), None)
    output, userobj = g_fifo_out.read_elem()

    # filter out all the objects/boxes that don't meet thresholds
    filtered_objs = filter_objects(output.astype(np.float32), input_image.shape[1], input_image.shape[0])

#    print('Displaying image with objects detected in GUI')
#    print('Click in the GUI window and hit any key to exit')
    #display the filtered objects/boxes in a GUI window
#    display_objects_in_gui(input_image, filtered_objs)
#    print('run 3')
#    cv2.imshow("111",input_image)
 #   cv2.waitKey(10)
 #   print('run end')
    return get_objects(input_image,filtered_objs)
def release():
    global g_device
    global g_graph
    global g_fifo_in
    global g_fifo_out
    print('release start')
    g_fifo_in.destroy()
    g_fifo_out.destroy()
    g_graph.destroy()
    g_device.close()
    g_device.destroy()
    print('release end')
def test_arg1(arg):
    print('func arg\n')
def test_arg(arg):
    print('func arg\n')
    #video_device = cv2.VideoCapture("rtsp://192.168.1.80:554/av0_1")
    #ret_val, input_image = video_device.read()
    cv2.imshow("123", arg)   
 
    raw_key = cv2.waitKey(20000)
    print('func arg done\ n')
def fun123():
    return 888
def fun4():
   # lst=(12,14,17,18)
    lst=tuple([123,2,3,4])
  #   lst=list([123,2,3,4])
    return lst 
    
def fun5():
   # lst=(12,14,17,18)
    tup2 = (1, 2, 3, 4, 5, 6, 7 )
  #   lst=list([123,2,3,4])
    return tup2     
def fun6():
   # lst=(12,14,17,18)
    list1 = [1, 2, 3, 4, 5, 6, 7 ]
    list2 =[22,33]
  #   lst=list([123,2,3,4])
    return list1+list2       
def func():
    print('func start')
    init()
    run()
    release()
    print('func end')
# This function is called from the entry point to do
# all the work.
def main():
    print('test start')
    func()
    return
    print('Running NCS Caffe TinyYolo example')

    # Set logging level to only log errors
    mvnc.global_set_option(mvnc.GlobalOption.RW_LOG_LEVEL, 3)
    devices = mvnc.enumerate_devices()
    if len(devices) == 0:
        print('No devices found')
        return 1
    device = mvnc.Device(devices[0])
    device.open()

    #Load graph from disk and allocate graph via API
    with open(tiny_yolo_graph_file, mode='rb') as f:
        graph_from_disk = f.read()
    graph = mvnc.Graph("Tiny Yolo Graph")
    fifo_in, fifo_out = graph.allocate_with_fifos(device, graph_from_disk)

    # Read image from file, resize it to network width and height
    # save a copy in display_image for display, then convert to float32, normalize (divide by 255),
    # and finally convert to convert to float16 to pass to LoadTensor as input for an inference
    cv_window_name='123'

    while (True):
        video_device = cv2.VideoCapture("./" + input_image_file)
        actual_frame_width = video_device.get(cv2.CAP_PROP_FRAME_WIDTH)
        actual_frame_height = video_device.get(cv2.CAP_PROP_FRAME_HEIGHT)
        print ('actual video resolution: ' + str(actual_frame_width) + ' x ' + str(actual_frame_height))
#        if ((video_device == None) or (not video_device.isOpened())):
#            print ('Could not open video device.  Make sure file exists:')
#            print ('file name:' + input_video_file)
#            print ('Also, if you installed python opencv via pip or pip3 you')
#            print ('need to uninstall it and install from source with -D WITH_V4L=ON')
#            print ('Use the provided script: install-opencv-from_source.sh')

       # frame_count = 0
       # start_time = time.time()

#frame_count = frame_count + 1

#frames_per_second = frame_count / (end_time - start_time)


        while True :
            print ('Use the provided script: install-opencv-from_source.sh')
            time1  = time.time()
            ret_val, input_image = video_device.read()

        time2  = time.time()

        start_time = time.time()


        display_image = input_image



        input_image = cv2.resize(input_image, (NETWORK_IMAGE_WIDTH, NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)
        input_image = input_image.astype(np.float32)
        input_image = np.divide(input_image, 255.0)
        input_image = input_image[:, :, ::-1]  # convert to RGB

        time3  = time.time()
        # Load tensor and get result.  This executes the inference on the NCS
        graph.queue_inference_with_fifo_elem(fifo_in, fifo_out, input_image.astype(np.float32), None)
        output, userobj = fifo_out.read_elem()

        # filter out all the objects/boxes that don't meet thresholds
        filtered_objs = filter_objects(output.astype(np.float32), input_image.shape[1], input_image.shape[0])

        print('Displaying image with objects detected in GUI')
        print('Click in the GUI window and hit any key to exit')
        #display the filtered objects/boxes in a GUI window
        display_objects_in_gui(display_image, filtered_objs)



        time4  = time.time()










        end_time = time.time()
        time_used=end_time-start_time
        print("time used : "+ str(time_used)+"time point:"+str(time1)+" "+str(time2)+" "+str(time3)+" "+str(time4))
        cv2.imshow(cv_window_name, input_image)
        #raw_key = cv2.waitKey(1)

        if (not ret_val):
             #   end_time = time.time()
                print("No image from from video device, exiting")
                break

             # resize image to network width and height
             # then convert to float32, normalize (divide by 255),
             # and finally convert to float16 to pass to LoadTensor as input
             # for an inference
       # input_image = cv2.resize(input_image, (TY_NETWORK_IMAGE_WIDTH, TY_NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)



             # save a display image as read from video device.

        # close video device
        video_device.release()














    input_image = cv2.imread(input_image_file)
    display_image = input_image



    input_image = cv2.resize(input_image, (NETWORK_IMAGE_WIDTH, NETWORK_IMAGE_HEIGHT), cv2.INTER_LINEAR)
    input_image = input_image.astype(np.float32)
    input_image = np.divide(input_image, 255.0)
    input_image = input_image[:, :, ::-1]  # convert to RGB

    # Load tensor and get result.  This executes the inference on the NCS
    graph.queue_inference_with_fifo_elem(fifo_in, fifo_out, input_image.astype(np.float32), None)
    output, userobj = fifo_out.read_elem()

    # filter out all the objects/boxes that don't meet thresholds
    filtered_objs = filter_objects(output.astype(np.float32), input_image.shape[1], input_image.shape[0])

    print('Displaying image with objects detected in GUI')
    print('Click in the GUI window and hit any key to exit')
    #display the filtered objects/boxes in a GUI window
    display_objects_in_gui(display_image, filtered_objs)

    fifo_in.destroy()
    fifo_out.destroy()
    graph.destroy()
    device.close()
    device.destroy()
    print('Finished')


# main entry point for program. we'll call main() to do what needs to be done.
if __name__ == "__main__":
    sys.exit(main())

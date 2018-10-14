import face_recognition
import cv2
import time
# This is a demo of running face recognition on live video from your webcam. It's a little more complicated than the
# other example, but it includes some basic performance tweaks to make things run a lot faster:
#   1. Process each video frame at 1/4 resolution (though still display it at full resolution)
#   2. Only detect faces in every other frame of video.

# PLEASE NOTE: This example requires OpenCV (the `cv2` library) to be installed only to read from your webcam.
# OpenCV is *not* required to use the face_recognition library. It's only required if you want to run this
# specific demo. If you have trouble installing it, try any of the other demos that don't require it instead.

# Get a reference to webcam #0 (the default one)
video_capture = cv2.VideoCapture(0)

# Initialize some variables
face_locations = []
face_encodings = []
face_names = []
process_this_frame = True

last_x = 0
last_y = 0

while True:
    # Grab a single frame of video
    ret, frame = video_capture.read()

    # Resize frame of video to 1/4 size for faster face recognition processing
    small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)

    # Convert the image from BGR color (which OpenCV uses) to RGB color (which face_recognition uses)
    rgb_small_frame = small_frame[:, :, ::-1]

    face_landmarks_list = face_recognition.face_landmarks(rgb_small_frame)

    for face_landmarks in face_landmarks_list:

        # Print the location of each facial feature in this image
        # for facial_feature in face_landmarks.keys():
        # print("The {} in this face has the following points: {}".format(facial_feature,
        #                                                                 face_landmarks[facial_feature]))

        # Let's trace out each facial feature in the image with a line!
        for facial_feature in face_landmarks.keys():
            for each_point in face_landmarks[facial_feature]:
                f = open("runtime/lip_position.txt", "w")
                if each_point[0] - last_x > 12:
                    f.write("%s left" % time.time())
                if each_point[0] - last_x < -12:
                    f.write("%s right" % time.time())
                if each_point[1] - last_y < -12:
                    f.write("%s top" % time.time())
                f.close()
                last_x = each_point[0]
                last_y = each_point[1]

    # Display the resulting image
    cv2.imshow('Video', small_frame)

    # Hit 'q' on the keyboard to quit!
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release handle to the webcam
video_capture.release()
cv2.destroyAllWindows()

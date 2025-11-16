import cv2
import mediapipe as mp
import socket
import time

# init MediaPipe Hands
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.5
)

# WiFi/Socket Configuration for ESP32 Access Point
ESP32_IP = "192.168.4.1"  # Default ESP32 AP IP address
ESP32_PORT = 8080

# Initialize socket connection
sock = None
last_sent_letter = None  # Track last sent letter to avoid duplicates

def connect_to_esp32():
    """Establish connection to ESP32 Access Point"""
    global sock
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((ESP32_IP, ESP32_PORT))
        print(f"✓ Connected to ESP32 at {ESP32_IP}:{ESP32_PORT}")
        return True
    except Exception as e:
        print(f"✗ Could not connect to ESP32: {e}")
        print("  Make sure you're connected to 'ASL_Translator' WiFi network")
        print("  Running in offline mode...")
        sock = None
        return False

def send_to_esp32(letter):
    """Send detected letter to ESP32"""
    global sock, last_sent_letter
    
    if sock is None:
        return False
    
    # spam prevention
    if letter == last_sent_letter:
        return False
    
    try:
        message = f"{letter}\n"
        sock.sendall(message.encode())
        last_sent_letter = letter
        print(f"→ Sent to ESP32: {letter}")
        return True
    except Exception as e:
        print(f"✗ Error sending to ESP32: {e}")
        sock = None
        return False

def detect_asl_letter(hand_landmarks):
    """
    Simple ASL detection based on finger positions
    Returns detected letter or None
    """
    # get landmarks
    landmarks = hand_landmarks.landmark
    
    # helper function to check if finger is extended
    def is_finger_extended(finger_tip, finger_pip):
        return finger_tip.y < finger_pip.y
    
    # thumb (special case - check x coordinate)
    # and also other fingers
    thumb_extended = landmarks[4].x < landmarks[3].x
    index_extended = is_finger_extended(landmarks[8], landmarks[6])
    middle_extended = is_finger_extended(landmarks[12], landmarks[10])
    ring_extended = is_finger_extended(landmarks[16], landmarks[14])
    pinky_extended = is_finger_extended(landmarks[20], landmarks[18])
    
    # count extended fingers
    extended_count = sum([
        thumb_extended,
        index_extended,
        middle_extended,
        ring_extended,
        pinky_extended
    ])
    
    # ASL letter recognition
    ## LETTERS
    if extended_count == 0:
        return 'A'
    elif not thumb_extended and index_extended and middle_extended and ring_extended and pinky_extended:
        return 'B'
    elif extended_count == 1 and thumb_extended:
        return 'C'
    elif extended_count == 1 and index_extended:
        return 'D'
    elif thumb_extended and index_extended and not middle_extended and not ring_extended and not pinky_extended:
        return 'L'
    elif extended_count == 2 and index_extended and middle_extended:
        return 'V'
    ## NUMBERS
    elif not thumb_extended and not index_extended and middle_extended and ring_extended and pinky_extended:
        return '3'
    elif thumb_extended and index_extended and middle_extended and ring_extended and pinky_extended:
        return '5'
    
    return None

# connect to ESP32 Access Point
print("\n" + "="*50)
print("ASL Translator - Connecting to ESP32")
print("="*50)
print("Step 1: Connect your computer to WiFi: 'ASL_Translator'")
print("Step 2: Password: 'asl12345'")
print("Step 3: Attempting connection...")
print("="*50 + "\n")

connect_to_esp32()

# webcam
cap = cv2.VideoCapture(0)
current_letter = None

print("ASL Detection Started. Press 'q' to quit.\n")

while cap.isOpened():
    success, frame = cap.read()
    if not success:
        break
    
    # Flip frame horizontally for mirror view
    frame = cv2.flip(frame, 1)
    
    # convert to RGB
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    
    # handle frame
    results = hands.process(rgb_frame)
    
    detected_letter = None
    
    # draw hand landmarks and detect letter
    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            # overlay draw landmarks
            mp_drawing.draw_landmarks(
                frame,
                hand_landmarks,
                mp_hands.HAND_CONNECTIONS
            )
            
            # detect ASL letter
            detected_letter = detect_asl_letter(hand_landmarks)
            
            # update current letter if detected
            if detected_letter and detected_letter != current_letter:
                current_letter = detected_letter
                print(f"✓ Detected: {current_letter}")
                
                # Send to ESP32
                send_to_esp32(current_letter)
    
    # display detected letter on screen
    connection_status = "Connected" if sock else "Offline"
    status_color = (0, 255, 0) if sock else (0, 0, 255)
    
    cv2.putText(frame, f"ESP32: {connection_status}", (10, frame.shape[0] - 20),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, status_color, 2)
    
    if current_letter:
        cv2.rectangle(frame, (5, 5), (250, 70), (255, 255, 255), -1)
        cv2.putText(frame, f"Letter: {current_letter}", (15, 45),
                    cv2.FONT_HERSHEY_SIMPLEX, 1.3, (0, 128, 0), 3)
    else:
        cv2.putText(frame, "Show hand gesture", (10, 50),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 2)
    
    # show frame
    cv2.imshow('ASL Detection', frame)
    
    # quit on 'q' press
    if cv2.waitKey(5) & 0xFF == ord('q'):
        break

# close sockets before quitting
print("\nShutting down...")
if sock:
    sock.close()
cap.release()
cv2.destroyAllWindows()
hands.close()

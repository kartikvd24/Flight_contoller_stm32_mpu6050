# 🧠 Edge AI Motion Classification using NanoEdge AI Studio

This project demonstrates motion **classification using 3-axis accelerometer data** (`accX`, `accY`, `accZ`) on an STM32 board.  
The model is built and tested using **NanoEdge AI Studio**, and sensor data is collected via **USB Serial Emulator** for training and validation.

---

## ⚙️ Project Overview

- **Goal:** Classify different motion types (e.g., rest, shake, tilt) using acceleration data.  
- **Hardware Used:** STM32 Nucleo Board (or compatible)  
- **Software Used:**
  - [NanoEdge AI Studio](https://nanoedgeaistudio.st.com/)
  - [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
  - [Tera Term](https://ttssh2.osdn.jp/)
  - Serial Emulator for testing (Virtual COM port)

---

## 🧩 Step 1: NanoEdge AI Studio Setup

1. Open **NanoEdge AI Studio** and create a new project.  
2. Select:
   - **Sensor Type:** Accelerometer  
   - **Data Input:** accX, accY, accZ
   - **Target:** STM32F401RE
3. Go to the Signals part and create new classes for the reuired entities for each class use "log from usb serial" and collect different singnals upto 500 signals i.e more the signals more the accuracy.
4.  then go to benchmark section and create new bench mark and use all the signals for traning.
5.  Then go for validation, in that use the best model that has been generated and use serial emulator just for testing after testing Deployment can be directly done using the generated code and dumping it directly on to the hardware.
 . Choose **Classification** as the AI function.  
. Configure **sampling frequency** (e.g., 50 Hz) and **window size** (e.g., 128 samples).  
. Import or record data from the **serial emulator** connected to STM32.

📘 **NanoEdge AI Studio Interface:**
![NanoEdge AI Studio](https://community.st.com/html/assets/legacy-nanoedge-ai-studio-1-4/Model_creation.jpg)

<img width="1920" height="1020" alt="image" src="https://github.com/user-attachments/assets/534cdb3e-375a-4a8a-88f4-74fe376ad685" />

<img width="1920" height="1020" alt="image" src="https://github.com/user-attachments/assets/738b0502-1b80-4c71-b0cf-165acc85e5eb" />

<img width="1920" height="1020" alt="image" src="https://github.com/user-attachments/assets/c4e6388d-8859-4845-a564-a3800da1b4fa" />

---

## 🧮 Step 2: USB Serial Data Logging

<img width="1920" height="1020" alt="image" src="https://github.com/user-attachments/assets/4fcd68fd-74af-4042-8bcf-7eebfa0e6dc5" />
This is done in the cubeide's terminal itself but you can use realterm for easier approach

To collect data, connect the STM32 via USB. The accelerometer values are sent using the serial port.

### Example Output:

---

## 🧰 Step 3: Serial Emulator Setup (Tera Term)

1. Connect STM32 via USB and open **Tera Term**.  
2. Select the correct **COM Port**.  
3. Set **Baud Rate** to **115200**, **Data bits:** 8, **Parity:** None, **Stop bits:** 1.  

---

## 📈 Step 4: Data Collection & Import into NanoEdge

- Use **Tera Term’s logging feature** to save accelerometer data as `.csv`.  
- Import the file into **NanoEdge AI Studio** for model training.  
- Label datasets for each motion class (e.g., still, tilt, shake).

📘 **Data Import Interface:**
![Data Import]<img width="1920" height="1020" alt="image" src="https://github.com/user-attachments/assets/afd7e60d-9b39-476b-9863-3d924f03a5e7" />


---

## 🧠 Step 5: Model Training & Testing

- NanoEdge AI Studio automatically tests multiple ML algorithms.
- Select the best performing model based on **accuracy** and **RAM/ROM usage**.
- Export the generated `.lib` file and integrate into STM32CubeIDE.



---

## 🧾 Step 6: Deploy and Test

- Include the generated **NanoEdge AI Library** in your project.  
- Stream new live accelerometer data over serial and view classification results in real-time.

---

## ✅ Results

| Motion | Predicted Class | Confidence |
|---------|----------------|-------------|
| Rest    | Stable          | 98% |
| left Tilt    |  left Tilt Detected  | 95% |
| flip   | flip Detected | 97% |
| right Tlit   |  Right Tilt Detected | 97% |
| up  | ascending | 100% |
| down   | decsending | 97% |

---

## 🧩 Conclusion

This project demonstrates how to use **NanoEdge AI Studio** and **serial data logging** for edge AI motion classification.  
Using `accX`, `accY`, and `accZ`, we trained and tested an embedded AI model capable of classifying human or device motion in real time — without needing cloud computing.

---

## 📚 References

- [NanoEdge AI Studio Documentation](https://wiki.st.com/stm32mcu/wiki/AI:NanoEdge_AI_Studio)
- [Tera Term Download Page](https://ttssh2.osdn.jp/)
- [STM32CubeIDE User Guide](https://www.st.com/en/development-tools/stm32cubeide.html)


Also there is a video attached in the files you can go through the working of the project.

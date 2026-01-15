
# TI-Glove-System: Triboelectric-Inertial Dual-Mode Sensing Glove

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)   [![Paper](https://img.shields.io/badge/Paper-Advanced%20Science-blue)](https://onlinelibrary.wiley.com/doi/10.1002/advs.202408689)   [![Python](https://img.shields.io/badge/Python-3.8%2B-blue)](https://www.python.org/)   [![Unity](https://img.shields.io/badge/Unity-2021.3%2B-black)](https://unity.com/)

**Official implementation code for the paper:**  
> **Triboelectric-Inertial Sensing Glove Enhanced by Charge-Retained Strategy for Human-Machine Interaction**  
> *Advanced Science, 2025*  
> **Authors:** Bo Yang, Jia Cheng, Xuecheng Qu, Yuning Song, Lifa Yang, Junyao Shen, Ziqian Bai, and Linhong Ji  
> **DOI:** [10.1002/advs.202408689](https://doi.org/10.1002/advs.202408689)

## 📖 Introduction
This repository contains the firmware, software algorithms, and application demos for the **TI-Glove**, a smart glove system integrating triboelectric bending sensors and inertial measurement units (IMU). The system features a novel charge-retained circuit for continuous motion capturing and achieves **99.38% accuracy** in sign language recognition using a 1D-CNN model.

![TI-Glove Diagram](./docs/fig1.png)

## 📂 Directory Structure

The project is organized as follows based on the system's functional modules:

```text
TI-Glove-System/
├── applications/                     # HMI Application Demos
│   ├── intuitive_interface/          # PC interface control (Zoom/Move) via gestures
│   ├── lighting_adjust/              # Smart home lighting control
│   │   ├── SendDemo_new/             # Arduino firmware (.ino) for IR control
│   │   └── WIFI_py_plot_new_light.py # Python control script
│   ├── robot_control/                # Robot hand/vehicle control
│   │   ├── firmware/                 # MCU code (4th.cpp) for robot (HC-12 module)
│   │   └── WIFI_py_plot_new_hand.py  # Python bridge script
│   └── vr_interaction/               # VR Application
│       ├── WIFI_py_plot_new_VR.py    # Python bridge for VR data forwarding
│       └── HandContrl2/              # Unity 3D Project for VR Interaction
│           ├── Assets/               # Core source: Scripts (C#), Scenes, Prefabs
│           ├── Packages/             # Unity dependencies
│           └── ProjectSettings/      # Project configuration
│
├── models/                           # Deep Learning for Sign Language Recognition
│   ├── data/                         # Dataset (CSV samples for 10 gestures)
│   ├── model_98.44.pt                # Pre-trained PyTorch model checkpoint
│   ├── main_CNN.py                   # Training and evaluation script
│   └── WIFI_py_plot_new_gesture.py   # Real-time inference script
│
├── server/                           # Data Acquisition & Visualization
│   ├── WIFI_py_plot_new_all.py       # Main server script to receive ESP32 Wi-Fi data
│   └── NetAssist.exe                 # Network debugging tool
│
└── docs/                             # Papers and Reference Manuals
```

## 🚀 Getting Started

### 1. Prerequisites

#### Hardware:
- TI-Glove (ESP32 + MPU9250 + TENG Sensors)
- PC with Wi-Fi capability
- Robot/Smart Light modules (for specific demos)

#### Software:
- Python 3.8+
- Unity 2021.3+ (for VR demos)
- Arduino IDE (for lighting/robot firmware)

### 2. Python Dependencies
Install the required libraries:

```bash
pip install numpy matplotlib torch pyserial
```

## 🛠 Usage Guide

### A. Data Acquisition (Server)
The `server` folder contains the core script to communicate with the TI-Glove via Wi-Fi.

Ensure your PC and the Glove (ESP32) are on the same Wi-Fi network.

Run the plotting server to visualize real-time signals:

```bash
python server/WIFI_py_plot_new_all.py
```

### B. Sign Language Recognition (Deep Learning)
Located in `models/`.

#### Dataset:
Raw CSV data for 10 gestures (Hello, Good, A, B, C...) is located in `models/data/`.

#### Real-time Recognition:
To test the pre-trained model (`model_98.44.pt`):

```bash
python models/WIFI_py_plot_new_gesture.py
```

#### Training:
To retrain the 1D-CNN model:

```bash
python models/main_CNN.py
```

### C. VR Interaction (Unity)
Located in `HandContrl2/`.

Run the Python bridge script to forward glove data:

```bash
python applications/vr_interaction/WIFI_py_plot_new_VR.py
```

Open Unity Hub and add the `HandContrl2` folder as a project.

Open the scene `Assets/Scenes/MainScene.unity`.

Press Play in Unity Editor to control the virtual hand.

### D. Robot & Smart Home

#### Robot Control:
- Flash `applications/robot_control/firmware/4th.cpp` to your robot's MCU.
- Run `python applications/robot_control/hand_control.py`.

#### Smart Lighting:
- Flash `applications/lighting_adjust/SendDemo_new/SendDemo_new.ino` to the Arduino controlling the lights.

- Run the control script:

```bash
python applications/lighting_adjust/WIFI_py_plot_new_light.py
```

## 🔗 Citation

If you find this work useful in your research, please cite our paper:

```bibtex
@article{Yang2025TIGlove,
  title = {Triboelectric-Inertial Sensing Glove Enhanced by Charge-Retained Strategy for Human-Machine Interaction},
  author = {Yang, Bo and Cheng, Jia and Qu, Xuecheng and Song, Yuning and Yang, Lifa and Shen, Junyao and Bai, Ziqian and Ji, Linhong},
  journal = {Advanced Science},
  year = {2025},
  doi = {10.1002/advs.202408689},
  publisher = {Wiley-VCH GmbH}
}
```

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

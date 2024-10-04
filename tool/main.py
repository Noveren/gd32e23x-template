
from typing import Literal
from pathlib import Path
from dataclasses import dataclass

import numpy as np
from numpy.typing import NDArray
import matplotlib.pyplot as plt
from matplotlib.axes import Axes
from scipy import signal

AVALIABLE_TYPES: list[str] = [
    "HP_YZ",
    "HP_CD",
]

AVALIABLE_LOWPASS: list[str] = [
    "All-Pass",
    "2kHz",
    "10kHz",
    "40kHz",
]

def voltage_int_to_float(x, *, max_voltage: float = 3.3, bit_num: int = 12):
    return x * max_voltage / (2 ** bit_num)

Fs = 100_000  # 采样频率

lowpass_2kHz = signal.butter(8, 2 * 2_000 / Fs, "lowpass")
lowpass_10kHz = signal.butter(8, 2 * 10_000 / Fs, "lowpass")
lowpass_40kHz = signal.butter(8, 2 * 40_000 / Fs, "lowpass")

def lowpass(x: NDArray, fc: Literal["2kHz", "10kHz", "40kHz"] = "40kHz") -> NDArray:
    b, a = None, None
    match fc:
        case "2kHz": b, a = lowpass_2kHz
        case "10kHz": b, a = lowpass_10kHz
        case "40kHz": b, a = lowpass_40kHz
    return signal.filtfilt(b, a, x)

def do(file: str, ty: str, lowpass: str) -> str | None:
    cwd_file = f"./{file}"
    if not (Path(cwd_file).exists() and Path(cwd_file).is_file()):
        return f"Unknowed file: {cwd_file}"
    if not ty in AVALIABLE_TYPES:
        return f"Undefined Type"
    if not lowpass in AVALIABLE_LOWPASS:
        return f"Undefined Lowpass"
    try:
        match ty:
            case "HP_YZ": HP_YZ(cwd_file, lowpass).show()
            case "HP_CD": HP_CD(cwd_file, lowpass).show()
    except Exception as e:
        print(e.with_traceback())
        return f"Failed to convert data"
    return None

@dataclass
class HP_YZ_Result:
    triger_idx: int
    mean_voltage: float
    voltages: NDArray
    switches: NDArray

class HP_YZ:
    """
        0. 需要数据字节数 % 4
        1. |  Ch0  Ch1 |
           | 0xXX 0xXX | ...
        2. [15] -> 1 未触发; 2 已触发; 所有通道
        3. [14] -> 惯性开关状态; 仅 Ch0
        4. [0-11] -> ADC
        5. [12-13] -> 三态开关
    """
    def __init__(self, file: str, lowpass: str):
        self.lowpass: str = lowpass
        bin16s = HP_YZ.get_bin16s(file)
        self.bin16s_0: NDArray = bin16s[0]
        self.bin16s_1: NDArray = bin16s[1]

    @staticmethod
    def get_bin16s(file: str) -> tuple[NDArray, NDArray]:
        fn_hex_to_int = np.vectorize(lambda x: int(x, base=16))
        bytes_str: list[str]
        with open(file, "r") as f:
            bytes_str = f.read().split()
        assert len(bytes_str) % 4 == 0
        bytes_arr: NDArray = np.array(bytes_str)
        bytes_arr = bytes_arr[0::2] + bytes_arr[1::2]
        bin16s: NDArray = fn_hex_to_int(bytes_arr).astype(np.uint16)
        bin16s_0: NDArray = bin16s[0::2]
        bin16s_1: NDArray = bin16s[1::2]
        return bin16s_0, bin16s_1
    
    @staticmethod
    def convert_channel(data: NDArray) -> HP_YZ_Result:
        bit_flag: NDArray = data & 0x8000
        triger_idx: int = int(np.sum(bit_flag == 0))
        idxs = np.where(((bit_flag[:-1] == 0x8000) & (bit_flag[1:] == 0x0000)) == True)[0]
        assert len(idxs) == 1
        idx: int = idxs[0]
        mean_voltage: float = voltage_int_to_float(data[idx+1] & 0x0FFF)
        segment_a = data[idx+2:]
        segment_b = data[:idx]
        data = np.hstack((segment_a, segment_b))
        voltages: NDArray = voltage_int_to_float(data & 0x0FFF)
        switches: NDArray = data & 0x4000
        switches[switches > 0] = 1
        return HP_YZ_Result(triger_idx, mean_voltage, voltages, switches)
    
    # @staticmethod
    # def convert_channel(data: NDArray) -> HP_YZ_Result:
    #     voltages: NDArray = voltage_int_to_float(data & 0x0FFF)
    #     switches: NDArray = data & 0x4000
    #     return HP_YZ_Result(10, 0, voltages, switches)
    
    @staticmethod
    def ax_design(ax: Axes, ch: HP_YZ_Result, lo: str, comment: str):
        ts: NDArray = np.arange(ch.voltages.shape[0]) / Fs
        ax.set_title(f"HP_YZ {comment}; Mean voltage ({ch.mean_voltage:.3f});")
        ax.set_ylim(0, 3.3)
        ax.grid()
        ax.plot([ts[ch.triger_idx], ts[ch.triger_idx]], [0, 3.3], c="r", label="Triger")
        ax.plot(ts, ch.voltages, label="origin")
        if lo != AVALIABLE_LOWPASS[0]:
            vs_lowpass = lowpass(ch.voltages, lo)
            ax.plot(ts, vs_lowpass, label=f"Software {lo}")
        ax.legend()
        ax: Axes = ax.twinx()
        ax.set_ylim(0, 2)
        ax.set_yticks((0, 1, 2))
        ax.plot(ts, ch.switches, c="black")
    
    def show(self):
        ch0 = HP_YZ.convert_channel(self.bin16s_0)
        ch1 = HP_YZ.convert_channel(self.bin16s_1)
        fig = plt.figure()
        NROWS: int = 2
        NCOLS: int = 1
        ax: Axes = fig.add_subplot(NROWS, NCOLS, 1)
        HP_YZ.ax_design(ax, ch0, self.lowpass, "Origin")
        ax: Axes = fig.add_subplot(NROWS, NCOLS, 2)
        HP_YZ.ax_design(ax, ch1, AVALIABLE_LOWPASS[0], "Hardware")
        plt.show()

@dataclass
class HP_CD_Result:
    triger_idx: int
    mean_voltage: float
    voltages: NDArray
    switches: NDArray

class HP_CD:
    """
        0. 需要数据字节数 % 8
        1. |  Ch0  Ch1  Ch2  Ch3 |
           | 0xXX 0xXX 0xXX 0xXX | ...
        2. [15] -> 1 未触发; 2 已触发; 所有通道
        3. [14] -> 惯性开关状态; 仅 Ch0
        4. [0-11] -> ADC
        5. [12-13] -> 三态开关
    """
    def __init__(self, file: str, lowpass: str):
        self.lowpass: str = lowpass
        bin16s = HP_CD.get_bin16s(file)
        self.bin16s_0: NDArray = bin16s[0]
        self.bin16s_1: NDArray = bin16s[1]
        self.bin16s_2: NDArray = bin16s[2]
        self.bin16s_3: NDArray = bin16s[3]

    @staticmethod
    def get_bin16s(file: str) -> tuple[NDArray, NDArray, NDArray, NDArray]:
        fn_hex_to_int = np.vectorize(lambda x: int(x, base=16))
        bytes_str: list[str]
        with open(file, "r") as f:
            bytes_str = f.read().split()
        assert len(bytes_str) % 8 == 0
        bytes_arr: NDArray = np.array(bytes_str)
        bytes_arr = bytes_arr[0::2] + bytes_arr[1::2]   # MLB
        # bytes_arr = bytes_arr[1::2] + bytes_arr[0::2]   # MSB
        bin16s: NDArray = fn_hex_to_int(bytes_arr).astype(np.uint16)
        bin16s_0: NDArray = bin16s[0::4]
        bin16s_1: NDArray = bin16s[1::4]
        bin16s_2: NDArray = bin16s[2::4]
        bin16s_3: NDArray = bin16s[3::4]
        return bin16s_0, bin16s_1, bin16s_2, bin16s_3
    
    @staticmethod
    def convert_channel(data: NDArray) -> HP_CD_Result:
        bit_flag: NDArray = data & 0x8000
        triger_idx: int = int(np.sum(bit_flag == 0))
        data = np.hstack((data[-triger_idx:], data[:-triger_idx]))
        # idxs = np.where(((bit_flag[:-1] == 0x8000) & (bit_flag[1:] == 0x0000)) == True)[0]
        # assert len(idxs) == 1
        # idx: int = idxs[0]
        # mean_voltage: float = voltage_int_to_float(data[idx+1] & 0x0FFF)
        # segment_a = data[idx:]
        # segment_b = data[:idx]
        # data = np.hstack((segment_a, segment_b))
        voltages: NDArray = voltage_int_to_float(data & 0x0FFF)
        switches: NDArray = data & 0x4000
        switches[switches > 0] = 1
        return HP_CD_Result(triger_idx, 0, voltages, switches)
    
    @staticmethod
    def ax_design(ax: Axes, ch: HP_CD_Result, lo: str, comment: str):
        ts: NDArray = np.arange(ch.voltages.shape[0]) / Fs
        ax.set_title(f"HP_CD {comment}; Mean voltage ({ch.mean_voltage:.3f});")
        ax.set_ylim(0, 3.3)
        ax.grid()
        ax.plot([ts[ch.triger_idx], ts[ch.triger_idx]], [0, 3.3], c="r", label="Triger")
        ax.plot(ts, ch.voltages, label="origin")
        if lo != AVALIABLE_LOWPASS[0]:
            vs_lowpass = lowpass(ch.voltages, lo)
            ax.plot(ts, vs_lowpass, label=f"Software {lo}")
        ax.legend()
        ax: Axes = ax.twinx()
        ax.set_ylim(0, 2)
        ax.set_yticks((0, 1, 2))
        # ax.plot(ts, ch.switches, c="black")
    
    def show(self):
        ch0 = HP_CD.convert_channel(self.bin16s_0)
        ch1 = HP_CD.convert_channel(self.bin16s_1)
        ch2 = HP_CD.convert_channel(self.bin16s_2)
        ch3 = HP_CD.convert_channel(self.bin16s_3)
        fig = plt.figure()
        NROWS: int = 2
        NCOLS: int = 2

        ax: Axes = fig.add_subplot(NROWS, NCOLS, 1)
        HP_CD.ax_design(ax, ch0, self.lowpass, "Ch0 Origin")
        ax: Axes = fig.add_subplot(NROWS, NCOLS, 2)
        HP_CD.ax_design(ax, ch1, AVALIABLE_LOWPASS[0], "CH1 Hardware")

        ax: Axes = fig.add_subplot(NROWS, NCOLS, 3)
        HP_CD.ax_design(ax, ch2, self.lowpass, "Ch2 Origin")
        ax: Axes = fig.add_subplot(NROWS, NCOLS, 4)
        HP_CD.ax_design(ax, ch3, AVALIABLE_LOWPASS[0], "CH3 Hardware")

        plt.show()

if __name__ == "__main__":
    res = HP_CD("input.txt", "All-Pass")
    res.show()

    # res = HP_CD("data_2024_07_24_12_15_34_65536.txt", "All-Pass")
    # res.show()
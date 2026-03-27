import json
import matplotlib.pyplot as plot

label_a = "No VoxelBits"
raw_json_a = r'''
{"time_ms": 2119.1506, "frames": 10, "chunks": 88, "world": "Test7-v1", "dynamic_state": "dynamic", "chunk_gpu_memory": 43970496, "chunk_cpu_memory": 92280320, "window_width": 1280, "window_height": 720}
'''

label_b = "VoxelBits"
raw_json_b = r'''
{"time_ms": 1589.9142, "frames": 10, "chunks": 88, "world": "Test7-v1", "dynamic_state": "dynamic", "chunk_gpu_memory": 44313696, "chunk_cpu_memory": 95163904, "window_width": 1280, "window_height": 720}
'''

data_a = json.loads(raw_json_a)
data_b = json.loads(raw_json_b)

if data_a["world"] != data_b["world"] or data_a["dynamic_state"] != data_b["dynamic_state"]:
    raise ValueError("World or Dynamic State mismatch between inputs")

def compute_metrics(d):
    return (
        d["chunk_gpu_memory"] / (1024 * 1024),
        d["chunk_cpu_memory"] / (1024 * 1024),
        d["time_ms"] / d["frames"]
    )

gpu_a, cpu_a, ft_a = compute_metrics(data_a)
gpu_b, cpu_b, ft_b = compute_metrics(data_b)

title_prefix = f'{data_a["world"].replace("-v1", "").title()} - {data_a["dynamic_state"].title()}'
labels = [label_a, label_b]

plot.figure()
plot.bar(labels, [gpu_a, gpu_b])
plot.ylabel("MB")
plot.title(f"{title_prefix} - GPU Memory Usage")
plot.tight_layout()
plot.savefig("gpu_memory.png")

plot.figure()
plot.bar(labels, [cpu_a, cpu_b])
plot.ylabel("MB")
plot.title(f"{title_prefix} - CPU Memory Usage")
plot.tight_layout()
plot.savefig("cpu_memory.png")

plot.figure()
plot.bar(labels, [ft_a, ft_b])
plot.ylabel("Milliseconds")
plot.title(f"{title_prefix} - {"Frame" if data_a["dynamic_state"] == "static" else "Meshing"} Time")
plot.tight_layout()
plot.savefig("frame_time.png")
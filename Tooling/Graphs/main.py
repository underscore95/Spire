import json
import matplotlib.pyplot as plot
from matplotlib.ticker import FuncFormatter

labels = [
    "Spire",
    "Minecraft"
]
# 237118828
# 1997
raw_jsons = [
    r'''
    {
        "time_ms": 398,
        "frames": 1000,
        "chunks": 384,
        "world": "Test8",
        "dynamic_state": "static",
        "chunk_gpu_memory": 65590080,
        "chunk_cpu_memory": 211739040,
        "window_width": 1280,
        "window_height": 720
    }
    ''',
    r'''
    {
        "time_ms": 1953.125,
        "frames": 1000,
        "chunks": 384,
        "world": "Test8",
        "dynamic_state": "static",
        "chunk_gpu_memory": 536870912,
        "chunk_cpu_memory": 730857472,
        "window_width": 1280,
        "window_height": 720
    }
    '''
]

if len(labels) != len(raw_jsons):
    raise ValueError("Labels and JSON list size mismatch")

data = [json.loads(r) for r in raw_jsons]

world = data[0]["world"]
dynamic_state = data[0]["dynamic_state"]

for d in data:
    if d["world"] != world or d["dynamic_state"] != dynamic_state:
        raise ValueError("World or Dynamic State mismatch between inputs")

def compute_metrics(d):
    return (
        d["chunk_gpu_memory"] / (1024 * 1024),
        d["chunk_cpu_memory"] / (1024 * 1024),
        d["time_ms"] / d["frames"]
    )

gpu_vals = []
cpu_vals = []
time_vals = []

for d in data:
    gpu, cpu, t = compute_metrics(d)
    gpu_vals.append(int(gpu))
    cpu_vals.append(int(cpu))
    time_vals.append(t)

title_prefix = f'{world.replace("-v1", "").title()} - {dynamic_state.title()}'

from matplotlib.ticker import FuncFormatter

formatter = FuncFormatter(lambda x, _: f"{int(x):,}")

plot.figure()
bars = plot.bar(labels, gpu_vals, color="crimson")
plot.ylabel("MB")
plot.title(f"{title_prefix} - GPU Memory Usage")
plot.gca().yaxis.set_major_formatter(formatter)
for bar, value in zip(bars, gpu_vals):
    plot.text(
        bar.get_x() + bar.get_width() / 2,
        value,
        f"{value:,}",
        ha="center",
        va="bottom"
    )
plot.tight_layout()
plot.savefig("gpu_memory.png")
plot.close()

plot.figure()
bars = plot.bar(labels, cpu_vals, color="green")
plot.ylabel("MB")
plot.title(f"{title_prefix} - CPU Memory Usage")
plot.gca().yaxis.set_major_formatter(formatter)
for bar, value in zip(bars, cpu_vals):
    plot.text(
        bar.get_x() + bar.get_width() / 2,
        value,
        f"{value:,}",
        ha="center",
        va="bottom"
    )
plot.tight_layout()
plot.savefig("cpu_memory.png")
plot.close()

plot.figure()
time_color = "#eedc5b" if dynamic_state == "static" else "purple"
bars = plot.bar(labels, time_vals, color=time_color)
plot.ylabel("Milliseconds")
#plot.yscale('log')
plot.title(f"{title_prefix} - {'Frame' if dynamic_state == 'static' else 'Meshing'} Time")
if dynamic_state == "static":
    plot.gca().yaxis.set_major_formatter(FuncFormatter(lambda x, _: f"{x:.3f}"))
else:
    plot.gca().yaxis.set_major_formatter(formatter)
for bar, value in zip(bars, time_vals):
    plot.text(
        bar.get_x() + bar.get_width() / 2,
        value,
        f"{value:.3f}" if dynamic_state == 'static' else f"{int(value):,}",
        ha="center",
        va="bottom"
    )
plot.tight_layout()
plot.savefig("frame_time.png")
plot.close()
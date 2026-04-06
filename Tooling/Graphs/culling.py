import json
import matplotlib.pyplot as plot
from matplotlib.ticker import FuncFormatter
from matplotlib.lines import Line2D

draw_line = True

raw_jsons = [
    r'''
    {
        "time_ms": 340,
        "frames": 1000,
        "chunks": 385,
        "world": "Test8",
        "dynamic_state": "static",
        "chunk_gpu_memory": 65590080,
        "chunk_cpu_memory": 211739040,
        "window_width": 1280,
        "window_height": 720,
        "num_rendered_faces": 610000,
        "num_faces": 1000000
    }
    ''',
    r'''
    {
        "time_ms": 340,
        "frames": 1000,
        "chunks": 385,
        "world": "Test8",
        "dynamic_state": "static",
        "chunk_gpu_memory": 65590080,
        "chunk_cpu_memory": 211739040,
        "window_width": 1280,
        "window_height": 720,
        "num_rendered_faces": 600000,
        "num_faces": 1000000
    }
    ''',
    r'''
    {
        "time_ms": 269,
        "frames": 1000,
        "chunks": 385,
        "world": "Test8",
        "dynamic_state": "static",
        "chunk_gpu_memory": 65590080,
        "chunk_cpu_memory": 211739040,
        "window_width": 1280,
        "window_height": 720,
        "num_rendered_faces": 370000,
        "num_faces": 1000000
    }
    '''
]

point_labels = [chr(ord('A') + i) for i in range(len(raw_jsons))]
labels = ["(A) Frustum", "(B) Back Face", "(C) Frustum + Back Face"]
#labels = point_labels

data = [json.loads(r) for r in raw_jsons]

world = data[0]["world"]
dynamic_state = data[0]["dynamic_state"]
num_faces_ref = data[0]["num_faces"]

for d in data:
    if d["world"] != world or d["dynamic_state"] != dynamic_state:
        raise ValueError("World or Dynamic State mismatch between inputs")
    if d["num_faces"] != num_faces_ref:
        raise ValueError("num_faces mismatch between inputs")
    if d["num_rendered_faces"] < 0 or d["num_rendered_faces"] > d["num_faces"]:
        raise ValueError("num_rendered_faces out of bounds")

def compute_metrics(d):
    gpu = d["chunk_gpu_memory"] / (1024 * 1024)
    cpu = d["chunk_cpu_memory"] / (1024 * 1024)
    time = d["time_ms"] / d["frames"]
    percent_culled = (1.0 - (d["num_rendered_faces"] / d["num_faces"])) * 100.0
    return gpu, cpu, time, percent_culled

gpu_vals = []
cpu_vals = []
time_vals = []
x_vals = []

for d in data:
    gpu, cpu, t, x = compute_metrics(d)
    gpu_vals.append(gpu)
    cpu_vals.append(cpu)
    time_vals.append(t)
    x_vals.append(x)

title_prefix = f'{world.replace("-v1", "").title()} - {dynamic_state.title()}'

formatter = FuncFormatter(lambda x, _: f"{int(x):,}")

def add_point_labels(ax, xs, ys, labels):
    y_range = max(ys) - min(ys) if max(ys) != min(ys) else 1
    offset = 0.02 * y_range
    for x, y, l in zip(xs, ys, labels):
        ax.text(x, y + offset, l, ha="center", va="bottom", fontsize=14, fontweight="bold")

def add_legend(ax, xs, vals, labels, unit):
    legend_labels = []
    for l, x, v in zip(labels, xs, vals):
        if unit == "MB":
            val_str = f"{int(v):,} MB"
        else:
            val_str = f"{v:,.3f} ms"
        legend_labels.append(f"{l}: {int(x)}% culled, {val_str}")

    handles = [Line2D([0], [0], color='none') for _ in legend_labels]
    ax.legend(handles, legend_labels, loc="best", frameon=True)

def plot_series(xs, ys, color):
    if draw_line:
        plot.plot(xs, ys, marker='o', color=color)
    else:
        plot.scatter(xs, ys, color=color)

plot.figure()
ax = plot.gca()
plot_series(x_vals, gpu_vals, "crimson")
ax.set_xlabel("% Culled")
ax.set_ylabel("MB")
ax.set_title(f"{title_prefix} - GPU Memory Usage")
ax.yaxis.set_major_formatter(formatter)
add_point_labels(ax, x_vals, gpu_vals, point_labels)
add_legend(ax, x_vals, gpu_vals, labels, "MB")
plot.tight_layout()
plot.savefig("gpu_memory.png")
plot.close()

plot.figure()
ax = plot.gca()
plot_series(x_vals, cpu_vals, "green")
ax.set_xlabel("% Culled")
ax.set_ylabel("MB")
ax.set_title(f"{title_prefix} - CPU Memory Usage")
ax.yaxis.set_major_formatter(formatter)
add_point_labels(ax, x_vals, cpu_vals, point_labels)
add_legend(ax, x_vals, cpu_vals, labels, "MB")
plot.tight_layout()
plot.savefig("cpu_memory.png")
plot.close()

plot.figure()
ax = plot.gca()
time_color = "#eedc5b" if dynamic_state == "static" else "purple"
plot_series(x_vals, time_vals, time_color)
ax.set_xlabel("% Culled")
ax.set_ylabel("Milliseconds")
ax.set_title(f"{title_prefix} - {'Frame' if dynamic_state == 'static' else 'Meshing'} Time")
if dynamic_state == "static":
    ax.yaxis.set_major_formatter(FuncFormatter(lambda x, _: f"{x:.3f}"))
else:
    ax.yaxis.set_major_formatter(formatter)
add_point_labels(ax, x_vals, time_vals, point_labels)
add_legend(ax, x_vals, time_vals, labels, "ms")
plot.tight_layout()
plot.savefig("frame_time.png")
plot.close()
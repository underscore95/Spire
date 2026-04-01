import matplotlib.pyplot as plot

labels = ["Instanced", "Spire"]
values = [600000, 1_863_000_000]

fig, ax = plot.subplots()
bars = ax.bar(labels, values)

ax.set_yscale("log")
plot.ylabel("Maximum Voxels at 60 FPS")

for bar, value in zip(bars, values):
    ax.text(
        bar.get_x() + bar.get_width() / 2,
        value,
        f"{value:,}",
        ha="center",
        va="bottom"
    )

plot.savefig("graph.png", bbox_inches="tight")
plot.close(fig)
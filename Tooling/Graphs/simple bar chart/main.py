import matplotlib.pyplot as plot

labels = ["No Culling", "Back Face Culling"]
values = [384, 384*6]

fig, ax = plot.subplots()
bars = ax.bar(labels, values)

#ax.set_yscale("log")
plot.ylabel("Draw Commands (Test8)")

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
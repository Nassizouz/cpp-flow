import os
import sys
import json
from collections import defaultdict

import matplotlib.pyplot as plt

def plot_graph(data, outpath):
  dataCount = range(len(data))
  fig, plots = plt.subplots(2)
  flowplot = plots[0].plot(dataCount, [ y["flow"] for y in data.values() ], ".-b", label= 'flow impl')
  classicplot = plots[0].plot(dataCount, [ y["classic"] for y in data.values() ], ".-r", label= 'classic impl')
  plots[0].set_xticks(dataCount)
  plots[0].set_xticklabels(list(data.keys()))
  plots[0].set_yscale("log")
  plots[0].legend()

  bars = plots[1].bar(
    dataCount, 
    # [ (y["classic"] / y["flow"]) - 1 if y["classic"] > y["flow"] else -((y["flow"] / y["classic"]) - 1) for y in data.values() ], 
    [ (y["classic"] - y["flow"]) / y["flow"] for y in data.values() ],
    tick_label = list(data.keys()))
  plots[1].bar_label(bars)
  curr_limits = plots[1].get_ylim()
  for bar, bench in zip(bars, data.values()):
    flowLoc, classicLoc = (1.5, 0.5) if bar.get_height() < 0 else (-1, -2)
    plots[1].text(bar.get_x(), flowLoc, f'{bench["flow"]:.0f}', fontsize= "small", color= 'b')
    plots[1].text(bar.get_x(), classicLoc, f'{bench["classic"]:.0f}', fontsize= "small", color= 'r')
  plots[1].set_ylim((curr_limits[0] - 3, curr_limits[1] + 3))

  fig.savefig(outpath)

benchmarks = None
with open(sys.argv[1], "r") as inputFile:
  benchmarks = json.load(inputFile)

validBenchmarks = defaultdict(lambda: defaultdict())
errorsBenchmarks = defaultdict(lambda: defaultdict())
for benchmark in benchmarks["benchmarks"]:
  benchInfo = json.loads(benchmark["name"])
  if benchInfo["valid_data"]:
    validBenchmarks[benchInfo["count"]][benchInfo["type"]] = benchmark["cpu_time"]
  else:
    errorsBenchmarks[benchInfo["count"]][benchInfo["type"]] = benchmark["cpu_time"]

plot_graph(validBenchmarks, os.path.join(sys.argv[2], "valid_data.png"))
plot_graph(errorsBenchmarks, os.path.join(sys.argv[2], "bad_data.png"))


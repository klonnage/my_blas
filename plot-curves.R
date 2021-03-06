times <- read.csv("times")
library(ggplot2)
mean <- aggregate(.~size, times[, 1-4], "mean")
min <- aggregate(.~size, times[, 1-4], "min")
max <- aggregate(.~size, times[, 1-4], "max")
stda <- aggregate(.~size, times[, 1-4], "sd")*sqrt(5/6)
names(stda)[names(stda) == "perf"] = "std"
stat <- cbind(mean, stda[, 2])
names(stat)[names(stat) == "stda[, 2]"] = "std"
print(stat)
curve <- ggplot(stat, aes(x = size, y = perf)) + 
	     geom_line() + 
	     geom_ribbon(aes(ymin = perf - std,
                       	     ymax = perf + std), alpha = 0.2)
print(curve + labs(x = "size : n", y = "performance (Flops/s)") + ggtitle("Performance for ``gemm\" on mistral02"))

#cover_min <- apply(mean, 2, "
#plot(mean, sub="Problem scaling", xlab="Vector size", ylab="GFlop/s", type="l", col="red")
#lines(min , sub="Problem scaling", xlab="Vector size", ylab="GFlop/s", type="l", col="blue")
#lines(max , sub="Problem scaling", xlab="Vector size", ylab="GFlop/s", type="l", col="green")
#lines(std , sub="Problem scaling", xlab="Vector size", ylab="GFlop/s", type="l", col="green")
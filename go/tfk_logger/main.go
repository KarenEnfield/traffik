// tfk_logger/main.go
package main

import "github.com/KarenEnfield/traffic/go/tfk_logger/logger"

func main() {
	var log = logger.NewLogger(logger.Info)
	log.Info("This is an info message from SomeFunction.")
	log.Warning("This is a warning message from SomeFunction.")
}

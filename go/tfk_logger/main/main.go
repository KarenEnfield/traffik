// tfk_logger/main.go
package main

import logger "github.com/KarenEnfield/traffik/go/tfk_logger/logger"

func main() {
	var log = logger.NewLogger("tfx_logger_main", logger.Info)
	log.Info("This is an info message from SomeFunction.")
	log.Warning("This is a warning message from SomeFunction.")
}

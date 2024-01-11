// tfk_logger/main.go
package main

import logger "github.com/KarenEnfield/traffik/go/tfk_logger/logger"

func main() {
	var lvl = logger.GetLogLevel()
	var log = logger.NewLogger("tfk_logger_main", lvl)
	log.Trace("This is a trace message from SomeFunction.")
	log.Debug("This is a debug message from SomeFunction.")
	log.Info("This is an info message from SomeFunction.")
	log.Warning("This is a warning message from SomeFunction.")
	log.Error("This is an error message from SomeFunction.")
	log.Critical("This is a critical message from SomeFunction.")

}

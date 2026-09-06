/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

void rrd_otel_log_write(const char *module, const char *format, ...)
{
     char message[512];
     FILE *log_file;
     va_list args;

     va_start(args, format);
     vsnprintf(message, sizeof(message), format, args);
     va_end(args);

     log_file = fopen("/opt/logs/remote_debugger_otel.log", "a");
     if (log_file == NULL)
     {
          return;
     }

     fprintf(log_file, "[%s] %s", module != NULL ? module : LOG_OTEL, message);
     fclose(log_file);
}

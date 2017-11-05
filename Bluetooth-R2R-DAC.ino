#include <TimerOne.h>
#include <SoftwareSerial.h>

#define SIGNALS_N 5
#define N 200

#define TYPE_PORT 0
#define TYPE_SIN 1
#define TYPE_SQUARE 2
#define TYPE_TRIANGLE 3
#define TYPE_SAW 4

SoftwareSerial mySerial(14, 15);

byte signal_s[SIGNALS_N * N]; // массив значений сигнала

int type = TYPE_TRIANGLE; // тип сигнала
int time_value = 9; // время задержки сигнала в мкс

// заполнение массива сигналов значениями этих сигналов
void signal_create() {
  for (int i = 0; i < N; i++) {
    signal_s[TYPE_PORT * N + N - 1 - i] = 255;
    signal_s[TYPE_SIN * N + N - 1 - i] = 127 * (sin(2 * M_PI / N * i) + 1); // синусоида
    signal_s[TYPE_SQUARE * N + N - 1 - i] = i < N / 2 ? 0 : 255; // прямоугольник
    signal_s[TYPE_TRIANGLE * N + N - 1 - i] = i < N / 2 ? (i * 255 / (N / 2)) : (N/2 - i) * 255 / (N/2 - 1) + 255; // треугольник
    signal_s[TYPE_SAW * N + N - 1 - i] = map(i, 0, N / 2, 0, 255);
  }
}

//
void signalDriver() {
  if (mySerial.available()) {
    char buf[128];

    int n = mySerial.readBytesUntil('\n', buf, sizeof(buf) - 1);

    buf[n] = '\0';
    char cmd[20];
    
    if (sscanf(buf, "%s %d", cmd, &time_value) == 2) {
      if (!strcmp(cmd, "port")) {
          type = TYPE_PORT;
          time_value %= 256;

          for (int i = 0; i < N; i++)
              signal_s[TYPE_PORT * N + i] = time_value;
          
          float u = 5000.0 / 256.0 * time_value;
          
          mySerial.print("port ");
          mySerial.print(time_value);
          mySerial.print(" (");
          mySerial.print(time_value, BIN);
          mySerial.print(", ");
          mySerial.print(u < 1000 ? u : u / 1000);
          mySerial.println(u < 1000 ? "mV)" : "V)");
      } else if (!strcmp(cmd, "sin")) {
          type = TYPE_SIN;
          
          mySerial.print("sin ");
          mySerial.println(time_value);
      } else if (!strcmp(cmd, "square")) {
          type = TYPE_SQUARE;
          
          mySerial.print("square ");
          mySerial.println(time_value);
      } else if (!strcmp(cmd, "triangle")) {
          type = TYPE_TRIANGLE;
          
          mySerial.print("triangle ");
          mySerial.println(time_value);
      }  else if (!strcmp(cmd, "saw")) {
          type = TYPE_SAW;
          
          mySerial.print("saw ");
          mySerial.println(time_value);
      } else {
        mySerial.println("Unknown command");
      }
    } else if (sscanf(buf, "%s", cmd) == 1) {
      if (!strcmp(cmd, "adc")) {
          type = TYPE_PORT;
          
          int v = analogRead(A5) / 4;
          float u = 5000.0 / 256.0 * v;
          
          mySerial.print("Analog value on A5: ");
          mySerial.print(v);
          mySerial.print(", ");
          mySerial.print(u < 1000 ? u : u / 1000);
          mySerial.println(u < 1000 ? "mV)" : "V)");
      } else
        mySerial.println("Unknown command");  
    }
    else
      mySerial.println("Unknown command");
  }
}

void setup() {
  signal_create();

  mySerial.begin(9600);

  mySerial.print("\n-------------------\n");
  mySerial.println("Hello from Arduino!");

  Timer1.initialize(500000);
  Timer1.attachInterrupt(signalDriver);

  DDRD = 0xff;
}

void loop() {
  int i = N;

  while (i--) {
    PORTD = signal_s[type * N + i];
    delayMicroseconds(time_value);
  }
}

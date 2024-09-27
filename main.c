#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <stdbool.h>

typedef enum {
  play,
  rec,
  playrec,
} Type;

typedef struct {
  char id; // id 0…
  char *name; // name eg "Sonova Consumer Hearing MOMENTUM 4"
  Type type; // type eg (play), (play/rec)
  bool def; // is the default
} Device;


Device *Devices[10] = {0};
int num_dev = 10;
int read_dev = 0;

int getDevices() {
  FILE *sndstat;
  char buffer[256];
  Device *dev;
  char *name_start = buffer + 7; 
  char *name_end;

  
  sndstat = fopen("/dev/sndstat", "r");
  if (sndstat == NULL) {
    perror("Error opening /dev/sndstat");
    return EXIT_FAILURE;
  }



// Parse lines of the following format:
// pcm4: <NAME OF MY HEADPHONES> (play/rec) default
//  id^   ^name                   ^type     ^selected

  while (fgets(buffer, sizeof(buffer), sndstat) != NULL) {
    name_end = strchr(buffer, '>');   
    if (name_end != NULL) {
      dev = malloc(sizeof(Device));
      size_t name_length = name_end -name_start;
      char *name = malloc(name_length);
      dev->id = buffer[3];
      strncpy(name, name_start, name_length);
      name[name_length] = '\0';
      dev->name = name;
      if (strcmp(name_end, " (play)") == 0) {
        dev->type = play;
      } else {
        dev->type = playrec;
      }

      dev->def = (strstr(buffer, "default") != NULL);

      Devices[read_dev] = dev;

      read_dev++;
    }
    
  }

  fclose(sndstat);

  return EXIT_SUCCESS;
}

int interactive() {
  return 0;
} 


void printList() {
  Device *dev;
  for (int i = 0; i < read_dev; i++) {
    dev = Devices[i];
    printf("%s %d\n", dev->name, dev->def);
  }
}

int setDefault(int id) {
  // The sysctl name for setting the default audio device
  const char *def_unit = "hw.snd.default_unit";
  int result;

  result = sysctlbyname(def_unit, NULL, NULL, &id, sizeof(id));
  if (result == -1) {
      perror("Failed to set audio device");
      return -1; // Return an error code
  }

  return 0;
}

void freeDevices() {
  Device *dev;
  for (int i = 0; i < read_dev; i++) {
    dev = Devices[i];
    free(dev->name);
    free(dev);
  }
}


int main(int argc, char *argv[]) {
  int result;
  if (getDevices() == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (argc >= 2) {

    if (strcmp(argv[1], "list") == 0) {
      printList();
    } else if (strcmp(argv[1], "setdef") == 0) {
      if (argc >= 3) {
        result = setDefault(atoi(argv[2]));
        if (result == 0) {
          printf("\x1b[1;31mSuccesfully changed default to %d", atoi(argv[2]));
        }
      } else {
        printf("\x1b[1;31mPlease provide the id for your preferred default device");
        result = EXIT_FAILURE;
      }
    }

  } else {
    result = interactive();
  }


 
  freeDevices();
  return EXIT_SUCCESS;
}

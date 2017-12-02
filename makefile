# main compiler
CXX := gcc

TARGET1 := Watering-Edit13


all: $(TARGET1)

$(TARGET1): 
	@echo "Compiling C program"
	$(CXX) $(CFLAGS) $(TARGET1).c -o $(TARGET1) $(LDFLAGS) -l$(LIB)

clean:
	@rm -rf $(TARGET1) $(TARGET2)

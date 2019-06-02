.PHONY: clean All

All:
	@echo "----------Building project:[ Snake - Release ]----------"
	@cd "Snake" && "$(MAKE)" -f  "Snake.mk"
clean:
	@echo "----------Cleaning project:[ Snake - Release ]----------"
	@cd "Snake" && "$(MAKE)" -f  "Snake.mk" clean

// main.cpp
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

// ========================
// Business Logic Classes
// ========================

struct Topping {
	std::string name;
	double price;
};

class MenuItem {
protected:
	std::string name;
	double basePrice;
	
public:
	MenuItem(const std::string& n, double p) : name(n), basePrice(p) {}
	virtual ~MenuItem() = default;
	
	std::string getName() const { return name; }
	double getBasePrice() const { return basePrice; }
	
	virtual double calculatePrice() const = 0;
	virtual void display() const = 0;
};

enum class PizzaSize { SMALL, MEDIUM, LARGE };
enum class BaseType { THIN, TRADITIONAL, THICK };

class Pizza : public MenuItem {
private:
	PizzaSize size;
	BaseType baseType;
	std::vector<Topping> toppings;
	
public:
	Pizza(const std::string& n, double p)
	: MenuItem(n, p), size(PizzaSize::MEDIUM), baseType(BaseType::THIN) {}
	
	void setSize(PizzaSize s) { size = s; }
	void setBaseType(BaseType bt) { baseType = bt; }
	void addTopping(const Topping& t) { toppings.push_back(t); }
	void removeTopping(const std::string& toppingName) {
		toppings.erase(std::remove_if(toppings.begin(), toppings.end(),
			[&](const Topping& t){ return t.name == toppingName; }), toppings.end());
	}
	
	double calculatePrice() const override {
		double price = basePrice;
		switch (size) {
			case PizzaSize::SMALL: price *= 1.0; break;
			case PizzaSize::MEDIUM: price *= 1.2; break;
			case PizzaSize::LARGE: price *= 1.5; break;
		}
		for (const auto& t : toppings) price += t.price;
		return price;
	}
	
	void display() const override {
		std::cout << "Pizza: " << name << ", Size: ";
		switch(size) {
			case PizzaSize::SMALL: std::cout << "Small"; break;
			case PizzaSize::MEDIUM: std::cout << "Medium"; break;
			case PizzaSize::LARGE: std::cout << "Large"; break;
		}
		std::cout << ", Base: ";
		switch(baseType) {
			case BaseType::THIN: std::cout << "Thin"; break;
			case BaseType::TRADITIONAL: std::cout << "Traditional"; break;
			case BaseType::THICK: std::cout << "Thick"; break;
		}
		std::cout << ", Toppings: ";
		for (const auto& t : toppings) std::cout << t.name << " ";
		std::cout << ", Price: $" << calculatePrice() << std::endl;
	}
	
	PizzaSize getSize() const { return size; }
	BaseType getBaseType() const { return baseType; }
	const std::vector<Topping>& getToppings() const { return toppings; }
};

class Drink : public MenuItem {
private:
	double volume;
	bool isCarbonated;
public:
	Drink(const std::string& n, double p, double v, bool c)
	: MenuItem(n, p), volume(v), isCarbonated(c) {}
	
	double calculatePrice() const override { return basePrice; }
	void display() const override {
		std::cout << "Drink: " << name << ", Volume: " << volume << "L"
		<< (isCarbonated ? ", Carbonated" : ", Non-carbonated")
		<< ", Price: $" << basePrice << std::endl;
	}
};

class SideDish : public MenuItem {
private:
	std::string portionSize;
public:
	SideDish(const std::string& n, double p, const std::string& portion)
	: MenuItem(n, p), portionSize(portion) {}
	
	double calculatePrice() const override { return basePrice; }
	void display() const override {
		std::cout << "SideDish: " << name << ", Portion: " << portionSize
		<< ", Price: $" << basePrice << std::endl;
	}
};

class Menu {
public:
	std::vector<Pizza> availablePizzas;
	std::vector<Drink> availableDrinks;
	std::vector<SideDish> availableSides;
	std::vector<Topping> availableToppings;
	
	Menu() {
		availablePizzas.emplace_back("Margherita", 5.0);
		availablePizzas.emplace_back("Pepperoni", 6.5);
		availablePizzas.emplace_back("Hawaiian", 7.0);
		
		availableDrinks.emplace_back("Coca-Cola", 1.5, 0.5, true);
		availableDrinks.emplace_back("Water", 1.0, 0.5, false);
		
		availableSides.emplace_back("French Fries", 2.0, "Medium");
		availableSides.emplace_back("Salad", 2.5, "Small");
		
		availableToppings.push_back({"Cheese", 0.5});
		availableToppings.push_back({"Mushrooms", 0.7});
		availableToppings.push_back({"Olives", 0.6});
		availableToppings.push_back({"Peppers", 0.4});
	}
};

enum class OrderType { DINE_IN, TAKEAWAY, DELIVERY };
enum class OrderStatus { PENDING, PREPARING, READY, DELIVERED, CANCELLED };

class Order {
public:
	int orderId;
	std::vector<Pizza> pizzas;
	std::vector<Drink> drinks;
	std::vector<SideDish> sides;
	
	std::string customerName;
	OrderType orderType;
	std::string deliveryAddress;
	double deliveryFee = 0;
	double totalAmount = 0;
	
	Order(int id) : orderId(id), orderType(OrderType::DINE_IN) {}
	
	void addPizza(const Pizza& p) { pizzas.push_back(p); }
	void addDrink(const Drink& d) { drinks.push_back(d); }
	void addSideDish(const SideDish& s) { sides.push_back(s); }
	
	void calculateTotal() {
		totalAmount = 0;
		for (const auto& p : pizzas) totalAmount += p.calculatePrice();
		for (const auto& d : drinks) totalAmount += d.calculatePrice();
		for (const auto& s : sides) totalAmount += s.calculatePrice();
		if (orderType == OrderType::DELIVERY) totalAmount += deliveryFee;
	}
	
	void displayOrder() const {
		std::cout << "Order #" << orderId << " for " << customerName << std::endl;
		std::cout << "Pizzas:\n";
		for (const auto& p : pizzas) p.display();
		std::cout << "Drinks:\n";
		for (const auto& d : drinks) d.display();
		std::cout << "Sides:\n";
		for (const auto& s : sides) s.display();
		std::cout << "Total: $" << totalAmount << std::endl;
	}
};

// ====================
// UI and Management
// ====================

enum class Screen { MAIN_MENU, CREATE_ORDER, VIEW_ORDER };

class PizzeriaApp {
private:
	Menu menu;
	std::vector<Order> activeOrders;
	int nextOrderId = 1;
	
	Screen currentScreen = Screen::MAIN_MENU;
	Order currentOrder = Order(0);
	
	Pizza tempPizza = Pizza("", 0);
	
	int selectedMenuCategory = 0;
	int selectedItemIndex = 0;
	
	int selectedToppingIndex = 0;
	
public:
	void run() {
		const int screenWidth = 900;
		const int screenHeight = 700;
		InitWindow(screenWidth, screenHeight, "Pizzeria Simulator");
		
		SetTargetFPS(60);
		
		currentOrder = Order(nextOrderId++);
		
		while (!WindowShouldClose()) {
			BeginDrawing();
			ClearBackground(RAYWHITE);
			
			switch (currentScreen) {
			case Screen::MAIN_MENU:
				drawMainMenu();
				break;
			case Screen::CREATE_ORDER:
				drawCreateOrder();
				break;
			case Screen::VIEW_ORDER:
				drawViewOrder();
				break;
			}
			
			EndDrawing();
		}
		
		CloseWindow();
	}
	
private:
	void drawMainMenu() {
		DrawText("Pizzeria Main Menu", 320, 40, 30, DARKGRAY);
		
		if (GuiButton({350, 150, 200, 50}, "Create New Order")) {
			currentOrder = Order(nextOrderId++);
			currentScreen = Screen::CREATE_ORDER;
		}
		
		if (GuiButton({350, 220, 200, 50}, "View Current Order")) {
			if (currentOrder.pizzas.empty() && currentOrder.drinks.empty() && currentOrder.sides.empty()) {
				// empty
			} else {
				currentScreen = Screen::VIEW_ORDER;
			}
		}
		
		if (GuiButton({350, 290, 200, 50}, "Exit")) {
			CloseWindow();
		}
	}
	
	void drawCreateOrder() {
		DrawText("Order Creation", 340, 20, 25, DARKGRAY);
		
		const char* categories[] = { "Pizzas", "Drinks", "Sides" };
		for (int i = 0; i < 3; i++) {
			Rectangle btnRect = {(float)(50 + i * 150), 70, 140, 40};
			if (GuiButton(btnRect, categories[i])) {
				selectedMenuCategory = i;
				selectedItemIndex = 0;
			}
			if (selectedMenuCategory == i) {
				DrawRectangleLinesEx(btnRect, 3, BLUE);
			}
		}
		
		int listY = 120;
		int count = 0;
		
		switch (selectedMenuCategory) {
		case 0:
			count = (int)menu.availablePizzas.size();
			for (int i = 0; i < count; i++) {
				std::string btnText = menu.availablePizzas[i].getName() + " $" + std::to_string(menu.availablePizzas[i].getBasePrice());
				Rectangle btnRect = {50, (float)(listY + i * 40), 300, 35};
				if (GuiButton(btnRect, btnText.c_str())) {
					tempPizza = menu.availablePizzas[i];
					tempPizza.setSize(PizzaSize::MEDIUM);
					tempPizza.setBaseType(BaseType::THIN);
					currentScreen = Screen::VIEW_ORDER;
					currentOrder.addPizza(tempPizza);
					currentOrder.calculateTotal();
				}
			}
			break;
		case 1:
			count = (int)menu.availableDrinks.size();
			for (int i = 0; i < count; i++) {
				std::string btnText = menu.availableDrinks[i].getName() + " $" + std::to_string(menu.availableDrinks[i].getBasePrice());
				Rectangle btnRect = {50, (float)(listY + i * 40), 300, 35};
				if (GuiButton(btnRect, btnText.c_str())) {
					currentOrder.addDrink(menu.availableDrinks[i]);
					currentOrder.calculateTotal();
				}
			}
			break;
		case 2:
			count = (int)menu.availableSides.size();
			for (int i = 0; i < count; i++) {
				std::string btnText = menu.availableSides[i].getName() + " $" + std::to_string(menu.availableSides[i].getBasePrice());
				Rectangle btnRect = {50, (float)(listY + i * 40), 300, 35};
				if (GuiButton(btnRect, btnText.c_str())) {
					currentOrder.addSideDish(menu.availableSides[i]);
					currentOrder.calculateTotal();
				}
			}
			break;
		}
		
		if (GuiButton({650, 600, 200, 50}, "Main Menu")) {
			currentScreen = Screen::MAIN_MENU;
		}
	}
	
	void drawViewOrder() {
		DrawText("Current Order", 350, 20, 25, DARKGRAY);
		
		int y = 80;
		DrawText("Pizzas:", 50, y, 20, BLACK);
		y += 30;
		for (const auto& p : currentOrder.pizzas) {
			std::string line = p.getName() + " $" + std::to_string(p.calculatePrice());
			DrawText(line.c_str(), 60, y, 18, DARKGRAY);
			y += 25;
		}
		
		y += 10;
		DrawText("Drinks:", 50, y, 20, BLACK);
		y += 30;
		for (const auto& d : currentOrder.drinks) {
			std::string line = d.getName() + " $" + std::to_string(d.calculatePrice());
			DrawText(line.c_str(), 60, y, 18, DARKGRAY);
			y += 25;
		}
		
		y += 10;
		DrawText("Sides:", 50, y, 20, BLACK);
		y += 30;
		for (const auto& s : currentOrder.sides) {
			std::string line = s.getName() + " $" + std::to_string(s.calculatePrice());
			DrawText(line.c_str(), 60, y, 18, DARKGRAY);
			y += 25;
		}
		
		y += 40;
		std::string totalStr = "Total: $" + std::to_string(currentOrder.totalAmount);
		DrawText(totalStr.c_str(), 50, y, 22, RED);
		
		if (GuiButton({650, 600, 200, 50}, "Back to Menu")) {
			currentScreen = Screen::MAIN_MENU;
		}
	}
};

int main() {
	PizzeriaApp app;
	app.run();
	return 0;
}

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <algorithm>
using namespace std;
//-----------------------------------------------------------------------------------
char DELIMETER = ':';
string REQUEST = "request_spot";
string ASSIGN = "assign_spot";
string CHECKOUT = "checkout";
string PASS_TIME = "pass_time";
string NORMAL = "normal";
string COVERED = "covered";
string CCTV = "CCTV";
string FREE_STATUS = "free";
string OCCUPIED_STATUS = "occupied";
const pair<int, int> PER_STATIC_PRICE_INCREASE_FOR_COVERED_CCTV[] = {
    {30, 50}, {60, 80}};
//----------------------------------------------------------------------------------
typedef int PARKING_ID;
typedef string CAR_NAME;
typedef int CAR_SIZE;
typedef int PRICE_SIZE;
struct PARKING_STRUCT
{
    int parking_size;
    string parking_type;
    string status = FREE_STATUS;
    int occupied_days = 0;
};
struct PRICE_STRUCT
{
    int static_price;
    int price_per_day;
};
typedef map<CAR_NAME, CAR_SIZE> CARS;
typedef map<PRICE_SIZE, PRICE_STRUCT> PRICES;
typedef map<PARKING_ID, PARKING_STRUCT> PARKINGS;
typedef map<string, PRICES> ALL_TYPE_PRICES; // 3 elements: 0=normal, 1= covered, 2= CCTV
typedef vector<vector<string>> SEPARATED_COLUMNS_DATA;
typedef pair<PARKING_ID, PARKING_STRUCT> PARKINGS_CONTENT;

//-----------------------------------------------------------------------------------

SEPARATED_COLUMNS_DATA get_data_func(string file_path)
{
    fstream csv_file;
    csv_file.open(file_path);
    if (csv_file.fail())
    {
        cout << "couldnt open file" << endl;
    }
    int columns_counter = 0;
    string line, word;
    SEPARATED_COLUMNS_DATA columns_data;
    int first_line_flag = 1;
    while (csv_file >> line)
    {
        stringstream line_stream(line);

        if (first_line_flag)
        {
            while (getline(line_stream, word, ','))
            {
                columns_data.push_back({word});
            }
            first_line_flag = 0;
            continue;
        }
        else
        {
            columns_counter = 0;
            while (getline(line_stream, word, ','))
            {
                columns_data[columns_counter].push_back(word);
                columns_counter++;
            }
        }
    }
    return columns_data;
}

void cars_data_func(CARS &cars, SEPARATED_COLUMNS_DATA separated_columns_data)
{
    for (int i = 1; i < separated_columns_data[0].size(); i++)
    {
        cars[separated_columns_data[0][i]] = stoi(separated_columns_data[1][i]);
    }
}

void parkings_data_func(PARKINGS &parkings, SEPARATED_COLUMNS_DATA separated_columns_data)
{
    PARKING_STRUCT parking_struct;
    parking_struct.occupied_days = 0;
    parking_struct.status = FREE_STATUS;
    for (int i = 1; i < separated_columns_data[0].size(); i++)
    {
        parking_struct.parking_size = stoi(separated_columns_data[1][i]);
        parking_struct.parking_type = separated_columns_data[2][i];
        parkings[stoi(separated_columns_data[0][i])] = parking_struct;
    }
}

void prices_data_func(PRICES &prices, SEPARATED_COLUMNS_DATA separated_columns_data)
{
    PRICE_STRUCT price_struct;
    for (int i = 1; i < separated_columns_data[0].size(); i++)
    {
        price_struct.static_price = stoi(separated_columns_data[1][i]);
        price_struct.price_per_day = stoi(separated_columns_data[2][i]);
        prices[stoi(separated_columns_data[0][i])] = price_struct;
    }
}

int find_car_size_func(CAR_NAME car_name, CARS cars)
{
    int result;
    for (auto it = cars.begin(); it != cars.end(); it++)
    {
        if (it->first == car_name)
        {
            result = it->second;
        }
    }
    return result;
}

void calculate_all_type_prices_func(PRICES prices, ALL_TYPE_PRICES &all_type_prices)
{
    PRICES temp_prices;
    PRICE_STRUCT temp_price_struct;
    int counter = 1;
    for (auto &&increase : PER_STATIC_PRICE_INCREASE_FOR_COVERED_CCTV)
    {
        for (auto it = prices.begin(); it != prices.end(); it++)
        {
            temp_price_struct.price_per_day = it->second.price_per_day + increase.first;
            temp_price_struct.static_price = it->second.static_price + increase.second;
            temp_prices[it->first] = temp_price_struct;
        }
        if (counter == 1)
        {
            all_type_prices[COVERED] = temp_prices;
        }
        else if (counter == 2)
        {
            all_type_prices[CCTV] = temp_prices;
        }
        counter++;
    }
}

string print_type_price_func(string parking_type, CAR_SIZE car_size, ALL_TYPE_PRICES all_type_prices, PARKINGS_CONTENT it)
{
    return to_string(all_type_prices[it.second.parking_type][car_size].static_price) + ' ' +
           to_string(all_type_prices[it.second.parking_type][car_size].price_per_day);
}

vector<PARKINGS_CONTENT> find_suitable_spots_func(CAR_SIZE car_size, ALL_TYPE_PRICES all_type_prices, PARKINGS &parkings)
{
    vector<PARKINGS_CONTENT> result;
    for (auto it = parkings.begin(); it != parkings.end(); it++)
    {
        if (it->second.parking_size == car_size && it->second.status == FREE_STATUS)
        {
            result.push_back(*it);
        }
    }
    return result;
}

template <typename I>
void print_suitable_spot_func(vector<I> suitable_spots_vector, CAR_SIZE car_size, ALL_TYPE_PRICES all_type_prices)
{
    for (auto it = suitable_spots_vector.begin(); it != suitable_spots_vector.end(); it++)
    {
        string parking_type = it->second.parking_type;
        cout << it->first << ": " << parking_type << ' ' << print_type_price_func(parking_type, car_size, all_type_prices, *it) << endl;
    }
}

void assending_spots_func(vector<PARKINGS_CONTENT> &suitable_spots_vector)
{
    sort(suitable_spots_vector.begin(), suitable_spots_vector.end(), [](PARKINGS_CONTENT &suitable_spots_content1, PARKINGS_CONTENT &suitable_spots_content2)
         { return (suitable_spots_content1).first < (suitable_spots_content2).first; });
}

void request_spot_func(CAR_NAME car_name, CARS cars, ALL_TYPE_PRICES all_type_prices, PARKINGS &parkings)
{

    int car_size = find_car_size_func(car_name, cars);
    vector<PARKINGS_CONTENT> suitable_spots_vector = find_suitable_spots_func(car_size, all_type_prices, parkings);
    assending_spots_func(suitable_spots_vector);
    print_suitable_spot_func(suitable_spots_vector, car_size, all_type_prices);
}

void assign_spot_func(PARKING_ID parking_id, PARKINGS &parkings)
{
    if (parkings[parking_id].status == FREE_STATUS)
    {
        parkings[parking_id].status = OCCUPIED_STATUS;
        cout << "Spot " << parking_id << " is occupied now." << endl;
    }
    else
    {
        cout << "error: choose another spot!" << endl;
    }
}

void time_settings_func(int days, PARKINGS &parkings)
{
    for (auto it = parkings.begin(); it != parkings.end(); it++)
    {
        if (it->second.status == OCCUPIED_STATUS)
        {
            it->second.occupied_days += days;
        }
    }
}

int find_per_price_func(string type, CAR_SIZE car_size, ALL_TYPE_PRICES all_type_prices)
{
    return all_type_prices[type][car_size].price_per_day;
}

int find_static_price_func(string type, CAR_SIZE car_size, ALL_TYPE_PRICES all_type_prices)
{
    return all_type_prices[type][car_size].static_price;
}

int total_cost_func(PARKING_ID parking_id, PARKINGS &parkings, ALL_TYPE_PRICES all_type_prices)
{
    int per_price = find_per_price_func(parkings[parking_id].parking_type, parkings[parking_id].parking_size, all_type_prices);
    int static_price = find_static_price_func(parkings[parking_id].parking_type, parkings[parking_id].parking_size, all_type_prices);
    return parkings[parking_id].occupied_days * per_price + static_price;
}

void checkout_func(PARKING_ID parking_id, PARKINGS &parkings, ALL_TYPE_PRICES all_type_prices)
{
    if (parkings[parking_id].status == OCCUPIED_STATUS)
    {
        parkings[parking_id].status = FREE_STATUS;
        cout << "Spot " << parking_id << " is free now." << endl;
        cout << "Total cost" << DELIMETER << " " << total_cost_func(parking_id, parkings, all_type_prices) << endl;
        parkings[parking_id].occupied_days = 0;
    }
}

void check_command_func(string user_command, PARKINGS &parkings, ALL_TYPE_PRICES all_type_prices, CARS &cars)
{
    vector<string> splitted_user_command;
    string tempstr;
    stringstream s(user_command);
    while (getline(s, tempstr, ' '))
    {
        splitted_user_command.push_back(tempstr);
    }

    if (splitted_user_command[0] == REQUEST)
    {
        request_spot_func(splitted_user_command[1], cars, all_type_prices, parkings);
    }
    else if (splitted_user_command[0] == ASSIGN)
    {
        assign_spot_func(stoi(splitted_user_command[1]), parkings);
    }
    else if (splitted_user_command[0] == CHECKOUT)
    {
        checkout_func(stoi(splitted_user_command[1]), parkings, all_type_prices);
    }
    else if (splitted_user_command[0] == PASS_TIME)
    {
        time_settings_func(stoi(splitted_user_command[1]), parkings);
    }
    else
    {
        cout << "undefined command!\n";
    }
}

void fill_data_structure_func(string car_path, string parkings_path, string prices_path, SEPARATED_COLUMNS_DATA separated_columns_data, PARKINGS &parkings, CARS &cars, PRICES &prices)
{
    separated_columns_data = get_data_func(car_path);
    cars_data_func(cars, separated_columns_data);
    separated_columns_data = get_data_func(parkings_path);
    parkings_data_func(parkings, separated_columns_data);
    separated_columns_data = get_data_func(prices_path);
    prices_data_func(prices, separated_columns_data);
}

//----------------------------

int main(int argc, char *argv[])
{
    CARS cars;
    PARKINGS parkings;
    PRICES prices;
    string car_path = argv[1];
    string parkings_path = argv[2];
    string prices_path = argv[3];
    SEPARATED_COLUMNS_DATA separated_columns_data;

    fill_data_structure_func(car_path, parkings_path, prices_path, separated_columns_data, parkings, cars, prices);

    ALL_TYPE_PRICES all_type_prices;
    all_type_prices[NORMAL] = prices;
    calculate_all_type_prices_func(prices, all_type_prices);

    string user_command;
    while (getline(cin, user_command))
    {
        check_command_func(user_command, parkings, all_type_prices, cars);
    }
    return 0;
}
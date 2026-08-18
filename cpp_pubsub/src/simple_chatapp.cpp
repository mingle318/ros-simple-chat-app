#include <memory>
#include <chrono>
#include <string>
#include <deque>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "cpp_pubsub/msg/message.hpp"

using namespace std;

void clear_screen() {
    cout << "\033[H\033[2J" << flush;
}


class ChatApp : public rclcpp::Node
{
public:
    ChatApp(string topic, string username)
    :Node("chatapp"), topic(topic),username(username) 
    {
        publisher_ = this->create_publisher<cpp_pubsub::msg::Message>(topic, 10);


        subscriber_ = this->create_subscription<cpp_pubsub::msg::Message>(
            topic, 
            10,
            bind(&ChatApp::subscriber_callback, this, placeholders::_1)
        );

    }
    void publish_message(string msg) {
        cpp_pubsub::msg::Message message;
        message.username = this->username;
        message.content = msg;
        publisher_->publish(message);
    }
private:
    rclcpp::Publisher<cpp_pubsub::msg::Message>::SharedPtr publisher_;
    rclcpp::Subscription<cpp_pubsub::msg::Message>::SharedPtr subscriber_;
    string topic, username;
    deque<pair<string, string>> message_list;
    void print_list() {
        for(auto [username, content]: message_list) {
            cout << "[" << username << "]: " << content << '\n';
        }
    }
    void subscriber_callback(cpp_pubsub::msg::Message::SharedPtr msg) {
        message_list.push_back(make_pair(msg->username, msg->content));
        if((int)message_list.size() > 10) message_list.pop_front();
        clear_screen();
        this->print_list();
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    string username;
    getline(cin, username);
    auto app = make_shared<ChatApp>("chat", username);
    thread input_thread([app]() {
        string input;
        while(rclcpp::ok()) {
            getline(cin, input);
            if(input == "QUIT") {
                rclcpp::shutdown();
                break;
            }
            app->publish_message(input);
        }
    });
    rclcpp::spin(app);
    input_thread.join();
    return 0;
}

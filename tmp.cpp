// server.Post("/users", [&collection](const httplib::Request &req, httplib::Response &res)
//             {
//         try {
//             json j = json::parse(req.body);
            
//             try{
//                 User newUser(j);  
//                 collection.insert_one(newUser.toBson().view());
//                 res.status = 201;
//                 std::cout << "beforw lang" << std::endl;
//                 res.set_content(newUser.toJson().dump(), "application/json");
//             }
//             catch(const std::exception& e){
//                 res.status = 400;
//                 res.set_content(make_error("Missing one of the required properties").dump(), "application/json");
//             }
//             // Добавляем в MongoDB
//         }
//         catch (const json::parse_error&) {
//             res.status = 400;
//             res.set_content(make_error("Invalid JSON format").dump(), "application/json");
//         }
//         catch (const std::exception& e) {
//             res.status = 500;
//             res.set_content(make_error(e.what()).dump(), "application/json");
//         } });

// User with all
// 
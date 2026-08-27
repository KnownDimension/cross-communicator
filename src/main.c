#include <stdio.h>
#include <unistd.h>


#include <concord/discord.h>





void on_ready(struct discord *client, const struct discord_ready * event) {

    struct discord_application_command_option channels[] = {
        {
            .type = DISCORD_APPLICATION_OPTION_CHANNEL,
            .name = "channel select",
            .description = "select the channel you want the bot to be active in",
            .required = true,
            .channel_types = DISCORD_CHANNEL_GUILD_TEXT,
        },



    };

    struct discord_create_guild_application_command params = {
        .name = "channel configure",
        .description = "configures channel to use for cross communication",
        .default_permission = true,
        .options =
            &(struct discord_application_command_options){
                .size = sizeof(channels) / sizeof * channels, // divides the bite size of the entire array by one element in bytes of that array
                .array = channels,
            },
    };



    discord_create_guild_application_command(client, g_app_id, event->guild_id, &params, NULL);


    }



}





void on_interaction_create(struct discord * client, const struct discord_interaction * event) {

        /* We're only interested on slash commands */
    if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) return;
    /* Return in case user input is missing for some reason */
    if (!event->data || !event->data->options) return;

    char * channelchosen = "tbd"


}






int main () {

    if (access("config.json", F_OK) == -1) { // checks if config file even exists
        printf("config.json does not exist, pls make one");
        return -1;
    }

    const char * config_file;
    config_file = "config.json"; // confic file name

    struct discord * client = discord_config_init(config_file); //  initialises a client based on the config file

    ccord_global_init(); // initialises shared resources

    discord_set_on_ready(client, &on_ready);
    discord_set_on_interaction_create(client, &on_interaction_create);



    discord_run(client); // run

    discord_cleanup(client); // cleanup when its all ended
    ccord_global_cleanup();




}

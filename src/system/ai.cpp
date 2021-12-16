#include "sand/component/movement_intent.hpp"
#include <algorithm>
#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <cmath>
#include <cstdlib>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/utility.hpp>
#include <sand/component/ai.hpp>
#include <sand/system/ai.hpp>
#include <utility>

void TickHuntersAI::operator()(entt::registry &registry) {
  registry.view<HunterAI, b2Body *>().each(
      [&registry, this](const auto hunter_entity, HunterAI &hunter_ai,
                        const b2Body *const hunter_body) {
        if (hunter_ai.state == HunterAI::Chase and
            b2DistanceSquared(
                hunter_body->GetPosition(),
                registry.get<b2Body *>(hunter_ai.target)->GetPosition()) <=
                this->chase_detection_range * this->chase_detection_range) {
          registry.get<HunterTarget>(hunter_ai.target).chased_by =
              hunter_entity;

          if (b2DistanceSquared(
                  registry.get<b2Body *>(hunter_ai.target)->GetPosition(),
                  hunter_body->GetPosition()) <=
              this->tag_range * this->tag_range) {
            registry.emplace_or_replace<TargetTagged>(
                hunter_ai.target, TargetTagged{.by = hunter_entity});

            registry.get<HunterTarget>(hunter_ai.target) =
                HunterTarget{.chased_by = hunter_ai.target};

            hunter_ai.state = HunterAI::Explore;

            hunter_ai.target = hunter_entity;
          }
          return;
        }

        auto hunter_position = hunter_body->GetPosition();

        std::vector<std::pair<entt::entity, int>> potential_targets;
        registry.view<HunterTarget, b2Body *>(entt::exclude<TargetTagged>)
            .each([this, hunter_position, &potential_targets, hunter_entity](
                      const auto target_entity, HunterTarget &target_data,
                      const b2Body *const target_body) {
              if (b2DistanceSquared(target_body->GetPosition(),
                                    hunter_position) <=
                      this->explore_detection_range *
                          this->explore_detection_range and
                  target_data.chased_by == target_entity) {

                potential_targets.push_back(std::make_pair(
                    target_entity, b2DistanceSquared(target_body->GetPosition(),
                                                     hunter_position)));
              }
            });
        if (potential_targets.size() == 0) {
          if (hunter_ai.target != hunter_entity)
            registry.get<HunterTarget>(hunter_ai.target).chased_by =
                hunter_ai.target;

          hunter_ai.state = HunterAI::Explore;
          hunter_ai.target = hunter_entity;
          if (hunter_ai.exlore_ticks <= 0) {
            hunter_ai.explore_target =
                b2Vec2(rand() % 60 + hunter_position.x - 30,
                       rand() % 60 + hunter_position.y - 30);
            hunter_ai.exlore_ticks = rand() % 50;
          } else
            hunter_ai.exlore_ticks--;

        } else if (potential_targets.size() >= 1) {
          std::sort(potential_targets.begin(), potential_targets.end(),
                    [](const auto left, const auto right) {
                      return left.second < right.second;
                    });

          const auto target_entity = potential_targets[0].first;
          if (potential_targets[0].second <=
              this->tag_range * this->tag_range) {
            registry.emplace_or_replace<TargetTagged>(
                target_entity, TargetTagged{.by = hunter_entity});

            registry.get<HunterTarget>(target_entity) =
                HunterTarget{.chased_by = target_entity};

            hunter_ai.state = HunterAI::Explore;

            hunter_ai.target = hunter_entity;

          } else {
            hunter_ai.target = target_entity;
            hunter_ai.state = HunterAI::Chase;
            registry.get<HunterTarget>(target_entity).chased_by = hunter_entity;
          }
        }
      });
}

void TickAIMovementIntent(entt::registry &registry) {
  registry.view<HunterAI, b2Body *>().each(
      [&registry](const entt::entity hunter_entity, const HunterAI &hunter_ai,
                  const b2Body *const hunter_body) {
        const auto hunter_position = hunter_body->GetPosition();
        if (hunter_ai.target != hunter_entity and
            registry.all_of<b2Body *>(hunter_ai.target)) {

          const auto target_position =
              registry.get<b2Body *>(hunter_ai.target)->GetPosition();

          registry.emplace_or_replace<MovementIntent>(
              hunter_entity,
              MovementIntent{.angle =
                                 atan2(target_position.y - hunter_position.y,
                                       target_position.x - hunter_position.x),
                             .velocity = 3});
        } else {

          registry.emplace_or_replace<MovementIntent>(
              hunter_entity,
              MovementIntent{
                  .angle =
                      atan2(hunter_position.y - hunter_ai.explore_target.y,
                            hunter_position.x - hunter_ai.explore_target.x),
                  .velocity = 1.5});
        }
      });
}
